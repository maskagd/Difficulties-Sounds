#include "sounds.hpp"

#include <Geode/Geode.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/utils/string.hpp>
#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

using namespace geode::prelude;

namespace {
    enum class SoundKey {
        NA,
        Easy,
        Normal,
        Hard,
        Harder,
        Insane,
        Demon,
        Auto,
        DemonAll,
        DemonEasy,
        DemonMedium,
        DemonHard,
        DemonInsane,
        DemonExtreme
    };

    enum class SoundPack {
        JG,
        Niko,
        Lobotomy,
        Eric
    };

    SoundPack getSelectedPack() {
        auto const pack = Mod::get()->getSettingValue<std::string>("sound-type");
        if (pack == "nnikzuu") {
            return SoundPack::Niko;
        }
        if (pack == "lobotomy") {
            return SoundPack::Lobotomy;
        }
        if (pack == "eric" || pack == "EricVanWilderman") {
            return SoundPack::Eric;
        }
        return SoundPack::JG;
    }

    float getVolume() {
        auto raw = Mod::get()->getSettingValue<int>("sound-volume");
        raw = std::clamp(raw, 0, 100);
        return static_cast<float>(raw) / 50.0f;
    }

    std::optional<std::string_view> getCustomSettingKey(SoundKey key) {
        switch (key) {
            case SoundKey::NA: return "na-sound";
            case SoundKey::Auto: return "auto-sound";
            case SoundKey::Easy: return "easy-sound";
            case SoundKey::Normal: return "normal-sound";
            case SoundKey::Hard: return "hard-sound";
            case SoundKey::Harder: return "harder-sound";
            case SoundKey::Insane: return "insane-sound";
            case SoundKey::DemonEasy: return "easydemon-sound";
            case SoundKey::DemonMedium: return "mediumdemon-sound";
            case SoundKey::DemonHard: return "harddemon-sound";
            case SoundKey::DemonInsane: return "insanedemon-sound";
            case SoundKey::DemonExtreme: return "extremedemon-sound";
            default: return std::nullopt;
        }
    }

    std::string difficultyStem(SoundKey key) {
        switch (key) {
            case SoundKey::NA: return "na";
            case SoundKey::Easy: return "easy";
            case SoundKey::Normal: return "normal";
            case SoundKey::Hard: return "hard";
            case SoundKey::Harder: return "harder";
            case SoundKey::Insane: return "insane";
            case SoundKey::Demon: return "harddemon";
            case SoundKey::Auto: return "auto";
            case SoundKey::DemonAll: return "harddemon";
            case SoundKey::DemonEasy: return "easydemon";
            case SoundKey::DemonMedium: return "mediumdemon";
            case SoundKey::DemonHard: return "harddemon";
            case SoundKey::DemonInsane: return "insanedemon";
            case SoundKey::DemonExtreme: return "extremedemon";
        }

        return "na";
    }

    std::string resolveBundledPath(SoundKey key) {
        auto const pack = getSelectedPack();
        auto const stem =
            pack == SoundPack::Niko ? "niko" :
            pack == SoundPack::Lobotomy ? "lobotomy" :
            pack == SoundPack::Eric ? "eric" :
            "jg";
        auto const extension = pack == SoundPack::Lobotomy ? ".mp3" : ".ogg";
        auto const rel = std::string(stem) + "." + difficultyStem(key) + extension;
        auto full = Mod::get()->expandSpriteName(rel);
        if (cocos::fileExistsInSearchPaths(full.c_str())) {
            return full;
        }
        log::warn("Bundled sound file not found: {}", rel);
        return full;
    }

    std::optional<std::string> resolveCustomPath(SoundKey key) {
        auto const customEnabled = Mod::get()->getSettingValue<bool>("sound-custom");
        if (!customEnabled) {
            return std::nullopt;
        }

        auto const customKey = getCustomSettingKey(key);
        if (!customKey.has_value()) {
            return std::nullopt;
        }

        auto const path = Mod::get()->getSettingValue<std::filesystem::path>(customKey.value());
        if (path.empty()) {
            return std::nullopt;
        }

        if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
            return std::nullopt;
        }

        return geode::utils::string::pathToString(path);
    }

    void playByKey(SoundKey key) {
        if (!Mod::get()->getSettingValue<bool>("sound-enable")) {
            return;
        }

        auto const volume = getVolume();
        if (volume <= 0.0f) {
            return;
        }

        auto const custom = resolveCustomPath(key);
        auto const path = custom.has_value() ? custom.value() : resolveBundledPath(key);

        FMODAudioEngine::get()->playEffect(path, 1.0f, 1.0f, volume);
    }

    std::optional<SoundKey> difficultySoundFromButton(std::string_view buttonID) {
        if (buttonID == "na-filter-button") return SoundKey::NA;
        if (buttonID == "easy-filter-button") return SoundKey::Easy;
        if (buttonID == "normal-filter-button") return SoundKey::Normal;
        if (buttonID == "hard-filter-button") return SoundKey::Hard;
        if (buttonID == "harder-filter-button") return SoundKey::Harder;
        if (buttonID == "insane-filter-button") return SoundKey::Insane;
        if (buttonID == "auto-filter-button") return SoundKey::Auto;
        return std::nullopt;
    }

    std::optional<SoundKey> demonSoundFromButton(std::string_view buttonID) {
        if (buttonID == "all-demon-filter-button") return SoundKey::DemonHard;
        if (buttonID == "easy-demon-filter-button") return SoundKey::DemonEasy;
        if (buttonID == "medium-demon-filter-button") return SoundKey::DemonMedium;
        if (buttonID == "hard-demon-filter-button") return SoundKey::DemonHard;
        if (buttonID == "insane-demon-filter-button") return SoundKey::DemonInsane;
        if (buttonID == "extreme-demon-filter-button") return SoundKey::DemonExtreme;
        return std::nullopt;
    }

    bool spriteDisplaysFrame(cocos2d::CCSprite* sprite, char const* frameName) {
        if (!sprite) {
            return false;
        }

        auto const cache = cocos2d::CCSpriteFrameCache::sharedSpriteFrameCache();
        if (!cache) {
            return false;
        }

        auto const frame = cache->spriteFrameByName(frameName);
        return frame && sprite->isFrameDisplayed(frame);
    }

    std::optional<SoundKey> demonSoundFromDifficultyFrame(cocos2d::CCSprite* sprite) {
        if (!sprite) {
            return std::nullopt;
        }

        if (spriteDisplaysFrame(sprite, "difficulty_07_btn2_001.png")) return SoundKey::DemonEasy;
        if (spriteDisplaysFrame(sprite, "difficulty_08_btn2_001.png")) return SoundKey::DemonMedium;
        if (spriteDisplaysFrame(sprite, "difficulty_06_btn2_001.png")) return SoundKey::DemonHard;
        if (spriteDisplaysFrame(sprite, "difficulty_09_btn2_001.png")) return SoundKey::DemonInsane;
        if (spriteDisplaysFrame(sprite, "difficulty_10_btn2_001.png")) return SoundKey::DemonExtreme;
        return std::nullopt;
    }

    std::optional<SoundKey> demonSoundFromDifficultyButton(cocos2d::CCNode* node) {
        if (!node) {
            return std::nullopt;
        }

        if (auto directSprite = typeinfo_cast<cocos2d::CCSprite*>(node->getChildByIndex(0))) {
            if (auto key = demonSoundFromDifficultyFrame(directSprite)) {
                return key;
            }
        }

        if (auto sprite = typeinfo_cast<cocos2d::CCSprite*>(node)) {
            if (auto key = demonSoundFromDifficultyFrame(sprite)) {
                return key;
            }
        }

        auto const children = node->getChildren();
        if (!children) {
            return std::nullopt;
        }

        for (auto child : geode::cocos::CCArrayExt<cocos2d::CCNode*, false>(children)) {
            if (auto key = demonSoundFromDifficultyButton(child)) {
                return key;
            }
        }

        return std::nullopt;
    }
}

std::optional<int> sounds::getDifficultyIndexForButton(std::string_view buttonID) {
    if (buttonID == "na-filter-button") return 0;
    if (buttonID == "easy-filter-button") return 1;
    if (buttonID == "normal-filter-button") return 2;
    if (buttonID == "hard-filter-button") return 3;
    if (buttonID == "harder-filter-button") return 4;
    if (buttonID == "insane-filter-button") return 5;
    if (buttonID == "demon-filter-button") return 6;
    if (buttonID == "auto-filter-button") return 7;
    return std::nullopt;
}

bool sounds::playForDifficultyButton(cocos2d::CCNode* buttonNode, std::string_view buttonID) {
    auto key = difficultySoundFromButton(buttonID);
    if (buttonID == "demon-filter-button") {
        key = demonSoundFromDifficultyButton(buttonNode).value_or(SoundKey::DemonHard);
    }

    if (!key.has_value()) {
        return false;
    }
    playByKey(key.value());
    return true;
}

bool sounds::playForDemonButton(std::string_view buttonID) {
    auto const key = demonSoundFromButton(buttonID);
    if (!key.has_value()) {
        return false;
    }
    playByKey(key.value());
    return true;
}
