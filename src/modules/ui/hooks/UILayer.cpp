// Letakkan di: src/modules/ui/hooks/UILayer.cpp
// Tombol emote di bawah tombol pause (saat main), list muncul ke bawah, game tetap jalan.
#include <globed/config.hpp>
#include <globed/core/EmoteManager.hpp>
#include <core/hooks/GJBaseGameLayer.hpp>
#include <modules/ui/UIModule.hpp>

#include <Geode/Geode.hpp>
#include <Geode/modify/UILayer.hpp>
#include <UIBuilder.hpp>
#include <cue/Util.hpp>
#include <vector>

using namespace geode::prelude;

namespace globed {

struct GLOBED_MODIFY_ATTR UIHookedUILayer : geode::Modify<UIHookedUILayer, UILayer> {
    static void onModify(auto& self) {
        UIModule::get().claimHooks(self);
    }

    struct Fields {
        CCMenu* m_btnMenu = nullptr;
        CCMenu* m_list = nullptr;
    };

    $override
    bool init(GJBaseGameLayer* gjbgl) {
        if (!UILayer::init(gjbgl)) return false;

        // hanya di level yang sedang aktif di Globed
        auto gpl = GlobedGJBGL::get();
        if (!gpl || !gpl->active()) return true;
        if (!globed::setting<bool>("core.player.quick-chat-enabled")) return true;

        auto winSize = CCDirector::get()->getWinSize();

        auto menu = CCMenu::create();
        menu->setID("quick-emote-menu"_spr);
        menu->setPosition({winSize.width - 30.f, winSize.height - 75.f}); // tepat di bawah tombol pause
        menu->setContentSize({0.f, 0.f});
        this->addChild(menu, 100);
        m_fields->m_btnMenu = menu;

        auto spr = CCSprite::create("icon-emotes.png"_spr);
        cue::rescaleToMatch(spr, 28.f);
        spr->setOpacity(150);

        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(UIHookedUILayer::onToggleEmotes));
        btn->setID("btn-quick-emote"_spr);
        menu->addChild(btn);

        return true;
    }

    void onToggleEmotes(CCObject*) {
        auto& f = *m_fields.self();

        if (f.m_list) {
            f.m_list->removeFromParent();
            f.m_list = nullptr;
            return;
        }

        constexpr float size = 28.f;
        constexpr float gap = 5.f;
        constexpr size_t cols = 2; // 2 kolom x 4 baris, supaya muat di layar
        auto& em = EmoteManager::get();

        std::vector<CCMenuItem*> items;

        for (uint32_t slot = 0; slot < 8; slot++) {
            auto emoteSpr = em.createFavoriteEmote(slot);
            if (!emoteSpr) continue; // slot kosong dilewati

            cue::rescaleToMatch(emoteSpr, size);

            auto item = CCMenuItemExt::createSpriteExtra(emoteSpr, [this, slot](CCObject*) {
                auto gpl = GlobedGJBGL::get();
                if (gpl) gpl->playSelfFavoriteEmote(slot); // ada cooldown 2.5 detik
                this->closeEmoteList();
            });
            items.push_back(item);
        }

        if (items.empty()) return; // belum ada emote favorit

        size_t rows = (items.size() + cols - 1) / cols;
        float width = cols * size + (cols - 1) * gap;
        float height = rows * size + (rows - 1) * gap;

        auto list = CCMenu::create();
        list->setID("quick-emote-list"_spr);
        list->ignoreAnchorPointForPosition(false);
        list->setContentSize({width, height});

        // posisi manual: kiri ke kanan, atas ke bawah (tanpa layout)
        for (size_t i = 0; i < items.size(); i++) {
            size_t row = i / cols;
            size_t col = i % cols;
            items[i]->setPosition({
                col * (size + gap) + size / 2.f,
                height - row * (size + gap) - size / 2.f
            });
            list->addChild(items[i]);
        }

        // kanan list sejajar dengan tepi kanan layar, tepat di bawah tombol emote
        list->setAnchorPoint({1.f, 1.f});
        list->setPosition({26.f, -22.f});

        f.m_btnMenu->addChild(list);
        f.m_list = list;
    }

    void closeEmoteList() {
        auto& f = *m_fields.self();
        if (f.m_list) {
            f.m_list->removeFromParent();
            f.m_list = nullptr;
        }
    }
};

}
