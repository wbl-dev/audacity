/*
 * Audacity: A Digital Audio Editor
 */

#include <algorithm>
#include <cstring>

#include <QCoreApplication>
#include <QFile>
#include <QTranslator>

#include <gtest/gtest.h>

#include "appshellcommands.h"
#include "internal/appshellcommandsregister.h"
#include "internal/applicationactioncontroller.h"
#include "internal/applicationuiactions.h"
#include "qml/Audacity/AppShell/appmenumodel.h"
#include "qml/Audacity/AppShell/aboutmodel.h"

#include "effects/effects_base/effectscommands.h"
#include "project/projectcommands.h"
#include "framework/diagnostics/diagnosticscommands.h"

namespace au::appshell {
namespace {

bool contains(const std::vector<muse::rcommand::Command>& commands, const muse::rcommand::Command& command)
{
    return std::find(commands.cbegin(), commands.cend(), command) != commands.cend();
}

bool contains(const muse::ui::UiActionList& actions, const muse::actions::ActionCode& actionCode)
{
    return std::find_if(actions.cbegin(), actions.cend(), [&actionCode](const muse::ui::UiAction& action) {
        return action.code == actionCode;
    }) != actions.cend();
}

class LinkedTranslatorCredits final : public QTranslator
{
public:
    QString translate(const char* context, const char* sourceText, const char*, int) const override
    {
        if (std::strcmp(context, "appshell/about") == 0 && std::strcmp(sourceText, "translator_credits") == 0) {
            return "<a href=\"mailto:review@example.invalid\">Review Translator</a><br>Plain Translator";
        }
        return {};
    }
};

TEST(workbench_online_surfaces_appshell, removes_about_dialog_external_links)
{

    LinkedTranslatorCredits translator;
    QCoreApplication::installTranslator(&translator);

    const AboutModel model;
    EXPECT_FALSE(model.externalLinksAvailable());

    bool foundTranslatedCredit = false;
    const QVariantList sections = model.creditList();
    for (const QVariant& sectionValue : sections) {
        const QVariantList credits = sectionValue.toMap().value("credits").toList();
        for (const QVariant& creditValue : credits) {
            const QVariantMap credit = creditValue.toMap();
            EXPECT_TRUE(credit.value("url").toString().isEmpty());
            EXPECT_FALSE(credit.value("raw").toString().contains("<a", Qt::CaseInsensitive));
            foundTranslatedCredit |= credit.value("raw").toString().contains("Review Translator");
        }
    }
    EXPECT_TRUE(foundTranslatedCredit);

    const QString gplText = model.gplText();
    EXPECT_TRUE(gplText.contains("GNU GENERAL PUBLIC LICENSE", Qt::CaseInsensitive));
    EXPECT_FALSE(gplText.contains("<a href", Qt::CaseInsensitive));

    QCoreApplication::removeTranslator(&translator);
}

} // namespace
} // namespace au::appshell
