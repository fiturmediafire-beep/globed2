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
        auto& em = EmoteManager::get();

        constexpr float gap = 5.f;

        auto list = CCMenu::create();
        list->setID("quick-emote-list"_spr);
        list->setLayout(ColumnLayout::create()
            ->setAxisReverse(true)      // urut dari atas ke bawah
            ->setAutoScale(false)
            ->setGap(gap)
            ->setAxisAlignment(AxisAlignment::End)
        );

        size_t count = 0;

        for (uint32_t slot = 0; slot < 8; slot++) {
            auto emoteSpr = em.createFavoriteEmote(slot);
            if (!emoteSpr) continue; // slot kosong dilewati

            cue::rescaleToMatch(emoteSpr, size);

            auto item = CCMenuItemExt::createSpriteExtra(emoteSpr, [this, slot](CCObject*) {
                auto gpl = GlobedGJBGL::get();
                if (gpl) gpl->playSelfFavoriteEmote(slot); // ada cooldown 2.5 detik
                this->closeEmoteList();
            });
            list->addChild(item);
            count++;
        }

        if (count == 0) return; // belum ada emote favorit

        // tinggi harus diisi, kalau 0 semua item menumpuk di satu titik
        list->setContentSize({size + 4.f, count * size + (count - 1) * gap});
        list->updateLayout();
        list->setAnchorPoint({0.5f, 1.f});
        list->setPosition({0.f, -22.f}); // di bawah tombol emote

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
