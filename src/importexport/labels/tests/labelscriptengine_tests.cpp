#include <gtest/gtest.h>

#include "importexport/labels/internal/labelscriptengine.h"
#include "importexport/labels/internal/labelparserregistry.h"
#include "importexport/labels/internal/au3/au3labelsimporter.h"
#include "importexport/labels/internal/labelsconfiguration.h"

#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QDir>

using namespace au::importexport;

namespace {
LabelScriptEngine workerEngine()
{
    return LabelScriptEngine(QString::fromUtf8(LABELSCRIPTWORKER_PATH));
}

LabelScriptInput sampleInput()
{
    return { "sample.ann", "ann", "ignored" };
}
}

TEST(LabelScriptEngineTests, AcceptsTypedLabelsAndIndependentFrequencyBounds)
{
    const QString script
        =
            R"(
        function parse(input) {
            return { trackName: input.fileName, labels: [
                { type: "point", start: 1.5, title: "A", highFrequency: 440 },
                { type: "range", start: 2, end: 3, title: "B", lowFrequency: 100 }
            ] };
        }
    )";
    const auto result = workerEngine().parse(script, sampleInput());
    ASSERT_TRUE(result.ret.success()) << result.ret.toString();
    ASSERT_EQ(result.val.labels.size(), 2);
    EXPECT_EQ(result.val.labels[0].kind, ScriptLabelKind::Point);
    EXPECT_DOUBLE_EQ(result.val.labels[0].start, result.val.labels[0].end);
    EXPECT_FALSE(result.val.labels[0].lowFrequency.has_value());
    EXPECT_DOUBLE_EQ(*result.val.labels[0].highFrequency, 440);
    EXPECT_EQ(result.val.labels[1].kind, ScriptLabelKind::Range);
    EXPECT_DOUBLE_EQ(result.val.labels[1].end, 3);
}

TEST(LabelScriptEngineTests, RejectsMissingKindAndZeroDurationRange)
{
    auto engine = workerEngine();
    EXPECT_FALSE(engine.parse("function parse(){return {trackName:'x',labels:[{start:1,title:'x'}]}}", sampleInput()).ret.success());
    EXPECT_FALSE(engine.parse("function parse(){return {trackName:'x',labels:[{type:'range',start:1,end:1,title:'x'}]}}",
                              sampleInput()).ret.success());
}

TEST(LabelScriptEngineTests, ReportsScriptExceptionAndInvalidFrequency)
{
    auto engine = workerEngine();
    const auto exception = engine.parse("function parse(){throw new Error('broken')} ", sampleInput());
    EXPECT_FALSE(exception.ret.success());
    EXPECT_NE(exception.ret.toString().find("broken"), std::string::npos) << exception.ret.toString();
    EXPECT_FALSE(engine.parse("function parse(){return {trackName:'x',labels:[{type:'point',start:1,title:'x',lowFrequency:-1}]}}",
                              sampleInput()).ret.success());
}

TEST(LabelScriptEngineTests, RejectsNonFiniteTimesAndMissingTitles)
{
    auto engine = workerEngine();
    EXPECT_FALSE(engine.parse("function parse(){return {trackName:'x',labels:[{type:'point',start:NaN,title:'x'}]}}",
                              sampleInput()).ret.success());
    EXPECT_FALSE(engine.parse("function parse(){return {trackName:'x',labels:[{type:'point',start:Infinity,title:'x'}]}}",
                              sampleInput()).ret.success());
    EXPECT_FALSE(engine.parse("function parse(){return {trackName:'x',labels:[{type:'point',start:1}]}}",
                              sampleInput()).ret.success());
}

TEST(LabelScriptEngineTests, KillsInfiniteLoop)
{
    const auto result = workerEngine().parse("function parse(){while(true){}}", sampleInput());
    EXPECT_FALSE(result.ret.success());
    EXPECT_NE(result.ret.toString().find("timed out"), std::string::npos);
}

TEST(LabelScriptEngineTests, RejectsOversizedOutput)
{
    const auto result = workerEngine().parse(
        "function parse(){return {trackName:'x',labels:[{type:'point',start:1,title:'x'.repeat(3000000)}]}}",
        sampleInput());
    EXPECT_FALSE(result.ret.success());
}

TEST(LabelParserRegistryTests, BuiltinTxtAcceptsMixedCaseSuffixAndFrequencies)
{
    LabelParserRegistry registry;
    const auto profile = registry.parserForSuffix(".TXT");
    ASSERT_TRUE(profile.has_value());
    ASSERT_EQ(registry.supportedExtensions(), std::vector<std::string> { "txt" });
    const LabelScriptInput input {
        "example.TXT", "txt", "1,5\tPoint\n\\\t-1\t440\n2\t3\tRange\n\\\t100\t-1\n"
    };
    const auto result = workerEngine().parse(profile->source, input);
    ASSERT_TRUE(result.ret.success()) << result.ret.toString();
    ASSERT_EQ(result.val.labels.size(), 2);
    EXPECT_EQ(result.val.labels[0].kind, ScriptLabelKind::Point);
    EXPECT_DOUBLE_EQ(result.val.labels[0].start, 1.5);
    EXPECT_FALSE(result.val.labels[0].lowFrequency.has_value());
    EXPECT_DOUBLE_EQ(*result.val.labels[0].highFrequency, 440);
    EXPECT_EQ(result.val.labels[1].kind, ScriptLabelKind::Range);
    EXPECT_DOUBLE_EQ(*result.val.labels[1].lowFrequency, 100);
    EXPECT_FALSE(result.val.labels[1].highFrequency.has_value());
}

TEST(LabelParserRegistryTests, MalformedTxtFailsWholeFileWithLineNumber)
{
    LabelParserRegistry registry;
    const auto profile = registry.parserForSuffix("txt");
    ASSERT_TRUE(profile.has_value());
    const LabelScriptInput input { "bad.txt", "txt", "1\tPoint\nnot-a-time\t2\tBad\n" };
    const auto result = workerEngine().parse(profile->source, input);
    EXPECT_FALSE(result.ret.success());
    EXPECT_NE(result.ret.toString().find("Line 2"), std::string::npos);
}

TEST(LabelParserRegistryTests, RejectsMalformedExtraFrequencyContinuation)
{
    LabelParserRegistry registry;
    const auto profile = registry.parserForSuffix("txt");
    ASSERT_TRUE(profile.has_value());
    const LabelScriptInput input { "bad.txt", "txt", "1\t1\tPoint\n\\\t100\t440\n\\\tbogus\t880\n" };
    const auto result = workerEngine().parse(profile->source, input);
    EXPECT_FALSE(result.ret.success());
    EXPECT_NE(result.ret.toString().find("Line 3"), std::string::npos);
}

TEST(LabelParserRegistryTests, RejectsInvalidUtf8BeforeProjectMutation)
{
    QTemporaryFile file(QDir::tempPath() + "/invalid-label-XXXXXX.txt");
    ASSERT_TRUE(file.open());
    ASSERT_EQ(file.write("\xff", 1), 1);
    file.close();
    Au3LabelsImporter importer({}, QString::fromUtf8(LABELSCRIPTWORKER_PATH));
    const muse::Ret result = importer.importData(muse::io::path_t(file.fileName()));
    EXPECT_FALSE(result.success());
    EXPECT_NE(result.toString().find("UTF-8"), std::string::npos);
}

TEST(LabelParserRegistryTests, RejectsOversizedFileBeforeReadingIntoEditor)
{
    QTemporaryFile file(QDir::tempPath() + "/oversized-label-XXXXXX.txt");
    ASSERT_TRUE(file.open());
    ASSERT_TRUE(file.resize(5 * 1024 * 1024));
    file.close();
    Au3LabelsImporter importer({}, QString::fromUtf8(LABELSCRIPTWORKER_PATH));
    const muse::Ret result = importer.importData(muse::io::path_t(file.fileName()));
    EXPECT_FALSE(result.success());
    EXPECT_NE(result.text().find("File exceeds 4 MiB limit"), std::string::npos);
}

TEST(LabelParserProfileTests, SavesAndReloadsNormalizedSuffixes)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = directory.filePath("parsers.json");
    LabelsConfiguration config(path);
    ASSERT_TRUE(config.saveParserProfile({ "one", "My parser", { ".ANN", "ann" }, true,
                                           "function parse(){}" }).success());
    LabelsConfiguration reloaded(path);
    ASSERT_EQ(reloaded.parserProfiles().size(), 1);
    EXPECT_EQ(reloaded.parserProfiles()[0].suffixes, QStringList { "ann" });
    LabelParserRegistry registry(&reloaded);
    ASSERT_TRUE(registry.parserForSuffix(".AnN").has_value());
    EXPECT_EQ(registry.parserForSuffix("ann")->id, "one");
}

TEST(LabelParserProfileTests, RejectsReservedAndConflictingSuffixesWithoutLosingValidProfile)
{
    QTemporaryDir directory;
    LabelsConfiguration config(directory.filePath("parsers.json"));
    ASSERT_TRUE(config.saveParserProfile({ "one", "One", { "ann" }, true, "function parse(){}" }).success());
    EXPECT_FALSE(config.saveParserProfile({ "two", "Two", { "ANN" }, true, "function parse(){}" }).success());
    EXPECT_FALSE(config.saveParserProfile({ "one", "One", { "wav" }, true, "function parse(){}" }).success());
    ASSERT_EQ(config.parserProfiles().size(), 1);
    EXPECT_EQ(config.parserProfiles()[0].suffixes, QStringList { "ann" });

    ASSERT_TRUE(config.saveParserProfile({ "two", "Two", { "ANN" }, false, "function parse(){}" }).success());
    LabelParserRegistry registry(&config);
    EXPECT_EQ(registry.parserForSuffix("ann")->id, "one");
    EXPECT_FALSE(config.saveParserProfile({ "two", "Two", { "ANN" }, true, "function parse(){}" }).success());
}

TEST(LabelParserProfileTests, RejectsProjectSuffixes)
{
    QTemporaryDir directory;
    LabelsConfiguration config(directory.filePath("parsers.json"));
    EXPECT_FALSE(config.saveParserProfile({ "three", "Project", { "aup3" }, true, "function parse(){}" }).success());
    EXPECT_FALSE(config.saveParserProfile({ "four", "Project", { "aup4unsaved" }, true, "function parse(){}" }).success());
}

TEST(LabelParserProfileTests, RejectsOversizedScriptSource)
{
    QTemporaryDir directory;
    LabelsConfiguration config(directory.filePath("parsers.json"));
    const QString source(1024 * 1024 + 1, QLatin1Char('x'));
    EXPECT_FALSE(config.saveParserProfile({ "huge", "Huge", { "ann" }, true, source }).success());
    EXPECT_TRUE(config.parserProfiles().empty());
}

TEST(LabelParserProfileTests, TxtOverrideRequiresExplicitConfirmation)
{
    QTemporaryDir directory;
    LabelsConfiguration config(directory.filePath("parsers.json"));
    const LabelParserProfile profile { "custom-txt", "Custom TXT", { "txt" }, true, "function parse(){}" };
    EXPECT_FALSE(config.saveParserProfile(profile).success());
    EXPECT_TRUE(config.saveParserProfile(profile, true).success());
    LabelParserRegistry registry(&config);
    ASSERT_TRUE(registry.parserForSuffix("TXT").has_value());
    EXPECT_EQ(registry.parserForSuffix("TXT")->id, "custom-txt");
}
