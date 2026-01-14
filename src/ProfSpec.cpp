/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "GossipDef.h"
#include "ScriptedGossip.h"
#include <sstream>

enum Specs
{
    ALCH_ELIXIR = 28677,
    ALCH_POTION = 28675,
    ALCH_TRANSMUTE = 28672,
    BSMITH_ARMOR = 9788,
    BSMITH_WEAPON = 9787,
    BSMITH_SWORD = 17039,
    BSMITH_AXE = 17041,
    BSMITH_HAMMER = 17040,
    ENG_GOBLIN = 20219,
    ENG_GNOME = 20222,
    LEATHER_DRAGON = 10656,
    LEATHER_ELEMENT = 10658,
    LEATHER_TRIBAL = 10660,
    TAILOR_MOON = 26798,
    TAILOR_SPELL = 26797,
    TAILOR_SHADOW = 26801
};

bool EnableAlch, EnableBSmith, EnableEng, EnableLeather, EnableTailor, EnableCost;
int speccost;

// Spec split by expansion (per design):
// Vanilla (Classic): Alchemy (Potion/Elixir/Transmutation), Blacksmithing (Armorsmith/Weaponsmith + Axe/Sword/Hammer),
// Engineering (Gnomish/Goblin), Leatherworking (Elemental/Tribal/Dragonscale)
// The Burning Crusade: Tailoring (Mooncloth/Shadoweave/Spellfire)
static uint8 GetRequiredLevelForSpec(uint32 specSpell)
{
    switch (specSpell)
    {
        case TAILOR_MOON:
        case TAILOR_SPELL:
        case TAILOR_SHADOW:
            return 70;
        default:
            return 60;
    }
}

static std::string FormatCostString(uint32 cost)
{
    uint32 gold = cost / 10000;
    uint32 silver = (cost / 100) % 100;
    uint32 copper = cost % 100;
    std::ostringstream oss;
    if (gold > 0)
        oss << gold << "g";
    if (silver > 0)
    {
        if (oss.tellp() > 0)
            oss << " ";
        oss << silver << "s";
    }
    if (copper > 0 || oss.tellp() == 0)
    {
        if (oss.tellp() > 0)
            oss << " ";
        oss << copper << "c";
    }
    return oss.str();
}

static std::string BuildConfirmText(std::string_view specName)
{
    if (EnableCost)
        return std::string("Purchase ") + std::string(specName) + " for " + FormatCostString(static_cast<uint32>(speccost)) + "?";
    return std::string("Learn ") + std::string(specName) + "?";
}


class ProfSpec : public CreatureScript
{
public:
    ProfSpec() : CreatureScript("npc_profession_specializations") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!sConfigMgr->GetOption<bool>("ProfSpec.Enable", true))
        {
            return false;
        }

        player->PlayerTalkClass->ClearMenus();

        if ((player->HasSkill(SKILL_ALCHEMY) && EnableAlch))
        {
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Alchemy", GOSSIP_SENDER_MAIN, SKILL_ALCHEMY);
        }
        if ((player->HasSkill(SKILL_BLACKSMITHING) && EnableBSmith))
        {
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Blacksmithing", GOSSIP_SENDER_MAIN, SKILL_BLACKSMITHING);
        }
        if ((player->HasSkill(SKILL_ENGINEERING) && EnableEng))
        {
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Engineering", GOSSIP_SENDER_MAIN, SKILL_ENGINEERING);
        }
        if ((player->HasSkill(SKILL_LEATHERWORKING) && EnableLeather))
        {
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Leatherworking", GOSSIP_SENDER_MAIN, SKILL_LEATHERWORKING);
        }
        if ((player->HasSkill(SKILL_TAILORING) && EnableTailor))
        {
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Tailoring", GOSSIP_SENDER_MAIN, SKILL_TAILORING);
        }


        SendGossipMenuFor(player, player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 Sender, uint32 SKILL)
    {
        player->PlayerTalkClass->ClearMenus();

        if (Sender == GOSSIP_SENDER_MAIN)
        {
            switch (SKILL)
            {
            case SKILL_ALCHEMY:
                if (player->GetLevel() < 60)
                {
                    ChatHandler(player->GetSession()).SendNotification("You must be at least level 60.");
                    CloseGossipMenuFor(player);
                    return true;
                }
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Elixir Master", GOSSIP_SENDER_INFO, ALCH_ELIXIR, BuildConfirmText("Elixir Master"), 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Potion Master", GOSSIP_SENDER_INFO, ALCH_POTION, BuildConfirmText("Potion Master"), 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Transmute Master", GOSSIP_SENDER_INFO, ALCH_TRANSMUTE, BuildConfirmText("Transmute Master"), 0, false);
                break;

            case SKILL_BLACKSMITHING:
                if (player->GetLevel() < 60)
                {
                    ChatHandler(player->GetSession()).SendNotification("You must be at least level 60.");
                    CloseGossipMenuFor(player);
                    return true;
                }
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Armorsmith", GOSSIP_SENDER_INFO, BSMITH_ARMOR, BuildConfirmText("Armorsmith"), 0, false);
                if (player->HasSpell(BSMITH_WEAPON))
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Master Swordsmith", GOSSIP_SENDER_INFO, BSMITH_SWORD, BuildConfirmText("Master Swordsmith"), 0, false);
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Master Axesmith", GOSSIP_SENDER_INFO, BSMITH_AXE, BuildConfirmText("Master Axesmith"), 0, false);
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Master Hammersmith", GOSSIP_SENDER_INFO, BSMITH_HAMMER, BuildConfirmText("Master Hammersmith"), 0, false);
                }
                else
                {
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Weaponsmith", GOSSIP_SENDER_INFO, BSMITH_WEAPON, BuildConfirmText("Weaponsmith"), 0, false);
                }
                break;

            case SKILL_ENGINEERING:
                if (player->GetLevel() < 60)
                {
                    ChatHandler(player->GetSession()).SendNotification("You must be at least level 60.");
                    CloseGossipMenuFor(player);
                    return true;
                }
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Gnomish Engineering", GOSSIP_SENDER_INFO, ENG_GNOME, BuildConfirmText("Gnomish Engineering"), 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Goblin Engineering", GOSSIP_SENDER_INFO, ENG_GOBLIN, BuildConfirmText("Goblin Engineering"), 0, false);
                break;

            case SKILL_LEATHERWORKING:
                if (player->GetLevel() < 60)
                {
                    ChatHandler(player->GetSession()).SendNotification("You must be at least level 60.");
                    CloseGossipMenuFor(player);
                    return true;
                }
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Dragonscale", GOSSIP_SENDER_INFO, LEATHER_DRAGON, BuildConfirmText("Dragonscale"), 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Elemental", GOSSIP_SENDER_INFO, LEATHER_ELEMENT, BuildConfirmText("Elemental"), 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Tribal", GOSSIP_SENDER_INFO, LEATHER_TRIBAL, BuildConfirmText("Tribal"), 0, false);
                break;

            case SKILL_TAILORING:
                if (player->GetLevel() < 70)
                {
                    ChatHandler(player->GetSession()).SendNotification("You must be at least level 70.");
                    CloseGossipMenuFor(player);
                    return true;
                }
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Shadoweave", GOSSIP_SENDER_INFO, TAILOR_SHADOW, BuildConfirmText("Shadoweave"), 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Mooncloth", GOSSIP_SENDER_INFO, TAILOR_MOON, BuildConfirmText("Mooncloth"), 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Spellfire", GOSSIP_SENDER_INFO, TAILOR_SPELL, BuildConfirmText("Spellfire"), 0, false);
                break;
            }

            SendGossipMenuFor(player, player->GetGossipTextId(creature), creature->GetGUID());
            return true;
        }

        if (Sender == GOSSIP_SENDER_INFO)
        {
            uint8 requiredLevel = GetRequiredLevelForSpec(SKILL);
            if (player->GetLevel() < requiredLevel)
            {
                ChatHandler(player->GetSession()).SendNotification("You must be at least level %u.", requiredLevel);
                CloseGossipMenuFor(player);
                return true;
            }
            if (player->HasSpell(SKILL))
            {
                ChatHandler(player->GetSession()).SendNotification("You already have this Specialization.");
            }
            else if (!EnableCost)
            {
                player->learnSpell(SKILL);
            }
            else
            {
                if (player->GetMoney() >= speccost)
                {
                    player->learnSpell(SKILL);
                    player->ModifyMoney(-speccost);
                }
                else
                {
                    ChatHandler(player->GetSession()).SendNotification("You do not have enough gold.");
                }
            }

            CloseGossipMenuFor(player);
            return true;
        }

        return false;
    }



};

class ProfSpecConf : public WorldScript
{
public:
    ProfSpecConf() : WorldScript("ProfSpecConf") {}

    void OnBeforeConfigLoad(bool) override
    {

        EnableAlch = sConfigMgr->GetOption<bool>("ProfSpec.Alchemy", true);
        EnableBSmith = sConfigMgr->GetOption<bool>("ProfSpec.Blacksmithing", true);
        EnableEng = sConfigMgr->GetOption<bool>("ProfSpec.Engineering", true);
        EnableLeather = sConfigMgr->GetOption<bool>("ProfSpec.Leatherworking", true);
        EnableTailor = sConfigMgr->GetOption<bool>("ProfSpec.Tailoring", true);
        EnableCost = sConfigMgr->GetOption<bool>("ProfSpec.CostEnabled", false);
        speccost = sConfigMgr->GetOption<int32>("ProfSpec.Cost", 10000);
    }
};



// Add all scripts in one
void AddProfSpec()
{
    new ProfSpec();
    new ProfSpecConf();
}
