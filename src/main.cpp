#include <Geode/Geode.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/modify/DemonFilterSelectLayer.hpp>
#include "sounds.hpp"
#include <optional>
#include <string>

using namespace geode::prelude;

class $modify(LevelSearchLayer) {
    void toggleDifficulty(CCObject* sender) {
        std::string buttonID;
        std::optional<int> diff;

        if (auto node = typeinfo_cast<CCNode*>(sender)) {
            buttonID = node->getID();
            diff = sounds::getDifficultyIndexForButton(buttonID);
        }

        auto const wasEnabled = diff.has_value() ? this->checkDiff(diff.value()) : false;
        LevelSearchLayer::toggleDifficulty(sender);

        if (diff.has_value() && !wasEnabled && this->checkDiff(diff.value())) {
            sounds::playForDifficultyButton(typeinfo_cast<CCNode*>(sender), buttonID);
        }
    }
};

class $modify(DemonFilterSelectLayer) {
    void selectRating(CCObject* sender) {
        auto const oldFilter = m_currentDemon;
        std::string buttonID;

        if (auto node = typeinfo_cast<CCNode*>(sender)) {
            buttonID = node->getID();
        }

        DemonFilterSelectLayer::selectRating(sender);

        if (m_currentDemon != oldFilter && !buttonID.empty()) {
            sounds::playForDemonButton(buttonID);
        }
    }
};
