#include "sounds.hpp"

#include <Geode/Geode.hpp>
#include <Geode/utils/cocos.hpp>

#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

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
    Niko
};

SoundPack getSelectedPack() {
    auto const pack = Mod::get()->getSettingValue<std::string>("sound-type");
    if (pack == "nnikzuu") {
        return SoundPack::Niko;
    }
    return SoundPack::JG;
}

float getVolume() {
    auto raw = Mod::get()->getSettingValue<int>("sound-volume");
    raw = std::clamp(raw, 0, 100);
    return static_cast<float>(raw) / 100.0f;
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

std::string stemForPack(SoundKey key, SoundPack pack) {
    if (pack == SoundPack::Niko) {
        switch (key) {
            case SoundKey::NA: return "nna";
            case SoundKey::Easy: return "neasy";
            case SoundKey::Normal: return "nnormal";
            case SoundKey::Hard: return "nhard";
            case SoundKey::Harder: return "nharder";
            case SoundKey::Insane: return "ninsane";
            case SoundKey::Demon: return "nhardd";
            case SoundKey::Auto: return "nauto";
            case SoundKey::DemonAll: return "nhardd";
            case SoundKey::DemonEasy: return "neasyd";
            case SoundKey::DemonMedium: return "nmediumd";
            case SoundKey::DemonHard: return "nhardd";
            case SoundKey::DemonInsane: return "ninsaned";
            case SoundKey::DemonExtreme: return "nextremed";
        }
    }

    switch (key) {
        case SoundKey::NA: return "na";
        case SoundKey::Easy: return "easy";
        case SoundKey::Normal: return "normal";
        case SoundKey::Hard: return "hard";
        case SoundKey::Harder: return "harder";
        case SoundKey::Insane: return "insane";
        case SoundKey::Demon: return "hardd";
        case SoundKey::Auto: return "auto";
        case SoundKey::DemonAll: return "hardd";
        case SoundKey::DemonEasy: return "easyd";
        case SoundKey::DemonMedium: return "mediumd";
        case SoundKey::DemonHard: return "hardd";
        case SoundKey::DemonInsane: return "insaned";
        case SoundKey::DemonExtreme: return "extremed";
    }

    return "na";
}

std::string makeCapitalizedExtremed(std::string const& stem) {
    if (stem == "extremed") {
        return "Extremed";
    }
    return stem;
}

std::string resolveBundledPath(SoundKey key) {
    auto const pack = getSelectedPack();
    auto const folder = pack == SoundPack::Niko ? "niko" : "jg";
    auto stem = stemForPack(key, pack);
    auto capitalizedStem = makeCapitalizedExtremed(stem);

    std::vector<std::string> candidates = {
        "audio/" + std::string(folder) + "/" + stem + ".ogg",
        "audio/" + std::string(folder) + "/" + capitalizedStem + ".ogg",
        std::string(folder) + "/" + stem + ".ogg",
        std::string(folder) + "/" + capitalizedStem + ".ogg",
        stem + ".ogg",
        capitalizedStem + ".ogg",
    };

    for (auto const& rel : candidates) {
        auto full = Mod::get()->expandSpriteName(rel);
        if (cocos::fileExistsInSearchPaths(full.c_str())) {
            return full;
        }
    }

    // Fallback if file lookup fails; FMOD may still resolve it.
    return Mod::get()->expandSpriteName(candidates.front());
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

    // File settings may hold placeholder text defaults; only use real files.
    if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
        return std::nullopt;
    }

    return path.string();
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
    if (buttonID == "demon-filter-button") return SoundKey::DemonHard;
    if (buttonID == "auto-filter-button") return SoundKey::Auto;
    return std::nullopt;
}

std::optional<SoundKey> demonSoundFromButton(std::string_view buttonID) {
    if (buttonID == "all-demon-filter-button") return SoundKey::DemonHard;
    if (buttonID == "easy-demon-filter-button") return SoundKey::DemonHard;
    if (buttonID == "medium-demon-filter-button") return SoundKey::DemonHard;
    if (buttonID == "hard-demon-filter-button") return SoundKey::DemonHard;
    if (buttonID == "insane-demon-filter-button") return SoundKey::DemonHard;
    if (buttonID == "extreme-demon-filter-button") return SoundKey::DemonHard;
    return std::nullopt;
}
} // namespace

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

bool sounds::playForDifficultyButton(std::string_view buttonID) {
    auto const key = difficultySoundFromButton(buttonID);
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

