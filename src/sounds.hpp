#pragma once
#include <cocos2d.h>
#include <optional>
#include <string_view>

namespace sounds {
    std::optional<int> getDifficultyIndexForButton(std::string_view buttonID);
    bool playForDifficultyButton(cocos2d::CCNode* buttonNode, std::string_view buttonID);
    bool playForDemonButton(std::string_view buttonID);
}
