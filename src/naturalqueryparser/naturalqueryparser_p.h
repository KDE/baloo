#ifndef __BALOO_NATURALQUERYPARSER_P_H__
#define __BALOO_NATURALQUERYPARSER_P_H__

#include "naturalqueryparser.h"
#include "patternmatcher.h"

namespace Baloo
{

template<typename T>
void NaturalQueryParser::runPass(const T &pass,
                                 int cursor_position,
                                 const QString &pattern,
                                 const KLocalizedString &description,
                                 CompletionProposal::Type type) const
{
    // Split the pattern at ";" characters, as a locale can have more than one
    // pattern that can be used for a given rule
    QStringList rules = pattern.split(QLatin1Char(';'));

    Q_FOREACH(const QString &rule, rules) {
        // Split the rule into parts that have to be matched
        QStringList parts = split(rule, false);
        PatternMatcher matcher(this, terms(), cursor_position, parts, type, description);

        matcher.runPass(pass);
    }
}

}

#endif
