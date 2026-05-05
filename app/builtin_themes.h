#pragma once

#include <vector>
#include "ui/core/UiTheme.h"

/**
 * @brief Returns all built-in themes parsed from flash-resident JSON strings.
 *
 * Built-in themes are defined as FSTR JSON literals in builtin_themes.cpp.
 * They are always available and serve as the baseline; user-edited copies
 * stored in ConfigDB shadow them by id in loadThemeSchemas().
 */
std::vector<lightinator::ui::core::UiTheme> loadBuiltinThemes();
