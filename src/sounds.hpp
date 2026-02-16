#pragma once

#include <optional>
#include <string_view>

namespace sounds {
std::optional<int> getDifficultyIndexForButton(std::string_view buttonID);
bool playForDifficultyButton(std::string_view buttonID);
bool playForDemonButton(std::string_view buttonID);
}
