/*
 * Audacity: A Digital Audio Editor
 */
#include "labelscriptworker.h"

#include <cstdio>
#include <cstdint>

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSEngine>
#include <QJSValue>

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
#include <sys/resource.h>
#endif

namespace au::importexport {
namespace {
constexpr uint32_t MAX_INPUT_BYTES = 4 * 1024 * 1024;
constexpr int MAX_OUTPUT_BYTES = 2 * 1024 * 1024;

bool readExact(void* destination, size_t count)
{
    auto* bytes = static_cast<unsigned char*>(destination);
    while (count > 0) {
        const size_t read = std::fread(bytes, 1, count, stdin);
        if (read == 0) {
            return false;
        }
        bytes += read;
        count -= read;
    }
    return true;
}

bool writeFrame(const QJsonObject& response)
{
    const QByteArray payload = QJsonDocument(response).toJson(QJsonDocument::Compact);
    if (payload.size() > MAX_OUTPUT_BYTES) {
        return false;
    }
    const uint32_t length = static_cast<uint32_t>(payload.size());
    const unsigned char header[4] = {
        static_cast<unsigned char>(length >> 24), static_cast<unsigned char>(length >> 16),
        static_cast<unsigned char>(length >> 8), static_cast<unsigned char>(length),
    };
    return std::fwrite(header, 1, 4, stdout) == 4
           && std::fwrite(payload.constData(), 1, payload.size(), stdout) == static_cast<size_t>(payload.size())
           && std::fflush(stdout) == 0;
}
}

int runLabelScriptWorker(int argc, char** argv)
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    // A runaway script is contained in this process. The host also enforces a wall clock deadline.
    rlimit limit {};
    limit.rlim_cur = limit.rlim_max = 512 * 1024 * 1024;
    const int memoryResource = RLIMIT_AS;
    if (setrlimit(memoryResource, &limit) != 0) {
        return 6;
    }
#endif
    QCoreApplication application(argc, argv);
    unsigned char header[4];
    if (!readExact(header, sizeof(header))) {
        return 2;
    }
    const uint32_t length = (uint32_t(header[0]) << 24) | (uint32_t(header[1]) << 16)
                            | (uint32_t(header[2]) << 8) | uint32_t(header[3]);
    if (length == 0 || length > MAX_INPUT_BYTES) {
        return 3;
    }
    QByteArray payload(static_cast<qsizetype>(length), Qt::Uninitialized);
    if (!readExact(payload.data(), length)) {
        return 4;
    }

    QJsonParseError parseError;
    const QJsonDocument request = QJsonDocument::fromJson(payload, &parseError);
    if (!request.isObject()) {
        writeFrame({ { "error", "Invalid worker request" } });
        return 0;
    }
    const QJsonObject requestObject = request.object();
    const QString source = requestObject.value("source").toString();
    const QJsonObject input = requestObject.value("input").toObject();

    QJSEngine engine;
    const QJSValue stringify = engine.evaluate("JSON.stringify");
    const QJSValue evaluated = engine.evaluate(source, input.value("fileName").toString());
    if (evaluated.isError()) {
        writeFrame({ { "error", evaluated.toString() } });
        return 0;
    }
    const QJSValue parser = engine.globalObject().property("parse");
    if (!parser.isCallable()) {
        writeFrame({ { "error", "Script must define parse(input)" } });
        return 0;
    }
    const QJSValue result = parser.call({ engine.toScriptValue(input.toVariantMap()) });
    if (result.isError()) {
        writeFrame({ { "error", result.toString() } });
        return 0;
    }
    const QJSValue serialized = stringify.call({ result });
    if (serialized.isError() || !serialized.isString()) {
        writeFrame({ { "error", "Parser result is not serializable JSON" } });
        return 0;
    }
    const QByteArray resultBytes = serialized.toString().toUtf8();
    if (resultBytes.size() > MAX_OUTPUT_BYTES - 64) {
        writeFrame({ { "error", "Parser result exceeds size limit" } });
        return 0;
    }
    const QJsonDocument resultDocument = QJsonDocument::fromJson(resultBytes, &parseError);
    if (!resultDocument.isObject()) {
        writeFrame({ { "error", "Parser must return an object" } });
        return 0;
    }
    return writeFrame({ { "result", resultDocument.object() } }) ? 0 : 5;
}
}
