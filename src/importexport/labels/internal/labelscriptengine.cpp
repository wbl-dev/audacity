/*
 * Audacity: A Digital Audio Editor
 */
#include "labelscriptengine.h"

#include <cmath>
#include <cstdint>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif
#if defined(Q_OS_MACOS)
#include <libproc.h>
#endif

namespace au::importexport {
namespace {
constexpr int MAX_INPUT_BYTES = 4 * 1024 * 1024;
constexpr int MAX_OUTPUT_BYTES = 2 * 1024 * 1024;
constexpr int WORKER_TIMEOUT_MS = 2000;
constexpr int MAX_LABELS = 100000;

muse::RetVal<ParsedLabelTrack> failure(const LabelScriptInput& input, const QString& message)
{
    return muse::RetVal<ParsedLabelTrack>::make_ret(muse::Ret::Code::BadData,
                                                    QString("%1: %2").arg(input.fileName, message).toStdString());
}

bool validNumber(const QJsonValue& value)
{
    return value.isDouble() && std::isfinite(value.toDouble()) && value.toDouble() >= 0.0;
}
}

LabelScriptEngine::LabelScriptEngine(QString workerExecutable)
    : m_workerExecutable(workerExecutable.isEmpty() ? QCoreApplication::applicationFilePath() : workerExecutable)
    , m_launchApplicationWorker(workerExecutable.isEmpty())
{
}

muse::RetVal<ParsedLabelTrack> LabelScriptEngine::parse(const QString& source, const LabelScriptInput& input) const
{
    const QJsonObject request {
        { "source", source },
        { "input", QJsonObject {
              { "fileName", input.fileName }, { "extension", input.extension }, { "text", input.text },
          } },
    };
    const QByteArray payload = QJsonDocument(request).toJson(QJsonDocument::Compact);
    if (payload.isEmpty() || payload.size() > MAX_INPUT_BYTES) {
        return failure(input, "Input exceeds size limit");
    }
    const uint32_t length = static_cast<uint32_t>(payload.size());
    QByteArray frame;
    frame.reserve(4 + payload.size());
    frame.append(static_cast<char>(length >> 24));
    frame.append(static_cast<char>(length >> 16));
    frame.append(static_cast<char>(length >> 8));
    frame.append(static_cast<char>(length));
    frame.append(payload);

    QProcess worker;
    worker.setProgram(m_workerExecutable);
    if (m_launchApplicationWorker) {
        worker.setArguments({ "--label-script-worker" });
    }
    worker.start();
    if (!worker.waitForStarted(1000)) {
        return failure(input, "JavaScript worker could not start");
    }
#if defined(Q_OS_WIN)
    HANDLE job = CreateJobObjectW(nullptr, nullptr);
    if (!job) {
        worker.kill();
        worker.waitForFinished(1000);
        return failure(input, "JavaScript worker memory limit could not be set");
    }
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits {};
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_MEMORY | JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    limits.ProcessMemoryLimit = 512 * 1024 * 1024;
    HANDLE process = OpenProcess(PROCESS_SET_QUOTA | PROCESS_TERMINATE, FALSE, static_cast<DWORD>(worker.processId()));
    const bool limited = process && SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits))
                         && AssignProcessToJobObject(job, process);
    if (process) {
        CloseHandle(process);
    }
    if (!limited) {
        CloseHandle(job);
        worker.kill();
        worker.waitForFinished(1000);
        return failure(input, "JavaScript worker memory limit could not be set");
    }
#endif
    worker.write(frame);
    worker.closeWriteChannel();
    QElapsedTimer elapsed;
    elapsed.start();
    bool exceededMemoryLimit = false;
    while (worker.state() != QProcess::NotRunning && elapsed.elapsed() < WORKER_TIMEOUT_MS) {
        worker.waitForFinished(20);
#if defined(Q_OS_MACOS)
        if (worker.state() != QProcess::NotRunning) {
            rusage_info_v2 usage {};
            if (proc_pid_rusage(static_cast<int>(worker.processId()), RUSAGE_INFO_V2,
                                reinterpret_cast<rusage_info_t*>(&usage)) != 0
                || usage.ri_resident_size > 512ULL * 1024 * 1024) {
                exceededMemoryLimit = true;
                break;
            }
        }
#endif
    }
    if (worker.state() != QProcess::NotRunning) {
        worker.kill();
        worker.waitForFinished(1000);
#if defined(Q_OS_WIN)
        CloseHandle(job);
#endif
        return failure(input, exceededMemoryLimit ? "JavaScript worker exceeded memory limit" : "JavaScript parser timed out");
    }
#if defined(Q_OS_WIN)
    CloseHandle(job);
#endif
    if (worker.exitStatus() != QProcess::NormalExit || worker.exitCode() != 0) {
        return failure(input, "JavaScript worker crashed or exceeded memory limit");
    }
    const QByteArray output = worker.readAllStandardOutput();
    if (output.size() < 4 || output.size() > MAX_OUTPUT_BYTES + 4) {
        return failure(input, "JavaScript worker output has invalid size");
    }
    const uint32_t outputLength = (uint32_t(static_cast<unsigned char>(output[0])) << 24)
                                  | (uint32_t(static_cast<unsigned char>(output[1])) << 16)
                                  | (uint32_t(static_cast<unsigned char>(output[2])) << 8)
                                  | uint32_t(static_cast<unsigned char>(output[3]));
    if (outputLength != static_cast<uint32_t>(output.size() - 4)) {
        return failure(input, "JavaScript worker output framing failed");
    }
    QJsonParseError jsonError;
    const QJsonDocument response = QJsonDocument::fromJson(output.mid(4), &jsonError);
    if (!response.isObject()) {
        return failure(input, "JavaScript worker returned invalid JSON");
    }
    const QJsonObject responseObject = response.object();
    if (responseObject.contains("error")) {
        return failure(input, responseObject.value("error").toString());
    }
    const QJsonObject result = responseObject.value("result").toObject();
    if (!result.value("trackName").isString() || !result.value("labels").isArray()) {
        return failure(input, "Parser must return { trackName, labels }");
    }
    const QJsonArray rows = result.value("labels").toArray();
    if (rows.isEmpty() || rows.size() > MAX_LABELS) {
        return failure(input, "Parser must return between 1 and 100000 labels");
    }
    ParsedLabelTrack parsed;
    parsed.trackName = result.value("trackName").toString();
    parsed.labels.reserve(rows.size());
    for (qsizetype i = 0; i < rows.size(); ++i) {
        const QJsonObject row = rows.at(i).toObject();
        const QString type = row.value("type").toString();
        const QString prefix = QString("Label %1: ").arg(i + 1);
        if (type != "point" && type != "range") {
            return failure(input, prefix + "type must be point or range");
        }
        if (!validNumber(row.value("start")) || !row.value("title").isString()) {
            return failure(input, prefix + "start or title is invalid");
        }
        ScriptLabel label;
        label.kind = type == "point" ? ScriptLabelKind::Point : ScriptLabelKind::Range;
        label.start = row.value("start").toDouble();
        label.end = label.start;
        label.title = row.value("title").toString();
        if (label.kind == ScriptLabelKind::Range) {
            if (!validNumber(row.value("end")) || row.value("end").toDouble() <= label.start) {
                return failure(input, prefix + "range end must be greater than start");
            }
            label.end = row.value("end").toDouble();
        } else if (row.contains("end")) {
            return failure(input, prefix + "point must not have an end");
        }
        if (row.contains("lowFrequency")) {
            if (!validNumber(row.value("lowFrequency"))) {
                return failure(input, prefix + "lowFrequency is invalid");
            }
            label.lowFrequency = row.value("lowFrequency").toDouble();
        }
        if (row.contains("highFrequency")) {
            if (!validNumber(row.value("highFrequency"))) {
                return failure(input, prefix + "highFrequency is invalid");
            }
            label.highFrequency = row.value("highFrequency").toDouble();
        }
        if (label.lowFrequency && label.highFrequency && *label.lowFrequency > *label.highFrequency) {
            return failure(input, prefix + "frequency bounds are reversed");
        }
        parsed.labels.push_back(std::move(label));
    }
    return muse::RetVal<ParsedLabelTrack>::make_ok(parsed);
}
}
