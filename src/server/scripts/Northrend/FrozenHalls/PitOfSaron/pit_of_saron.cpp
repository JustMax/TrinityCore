/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptPCH.h"
#include "pit_of_saron.h"
#include "Vehicle.h"

enum Spells
{
    SPELL_FIREBALL              = 69583, //Ymirjar Flamebearer
    SPELL_HELLFIRE              = 69586,
    SPELL_TACTICAL_BLINK        = 69584,
    SPELL_FROST_BREATH          = 69527, // Iceborn Proto-Drake
    SPELL_BLINDING_DIRT         = 70302, // Wrathbone Laborer
    SPELL_PUNCTURE_WOUND        = 70278,
    SPELL_SHOVELLED             = 69572,
    SPELL_LEAPING_FACE_MAUL     = 69504, // Geist Ambusher
    SPELL_SUMMON_ICICLE         = 69424,
    SPELL_ICICLE_VISUAL         = 69426,
    SPELL_ICICLE_DAMAGE_TRIGGER = 69428,
};

enum Events
{
    // Ymirjar Flamebearer
    EVENT_FIREBALL              = 1,
    EVENT_TACTICAL_BLINK        = 2,

    // Wrathbone Laborer
    EVENT_BLINDING_DIRT         = 3,
    EVENT_PUNCTURE_WOUND        = 4,
    EVENT_SHOVELLED             = 5,
};

enum Actions
{
    // Invisible Stalker (All phases)
    ACTION_COLLAPSE_ICICLE      = 1,
};

class mob_ymirjar_flamebearer : public CreatureScript
{
    public:
        mob_ymirjar_flamebearer() : CreatureScript("mob_ymirjar_flamebearer") { }

        struct mob_ymirjar_flamebearerAI: public ScriptedAI
        {
            mob_ymirjar_flamebearerAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void Reset()
            {
                _events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                _events.ScheduleEvent(EVENT_FIREBALL, 4000);
                _events.ScheduleEvent(EVENT_TACTICAL_BLINK, 15000);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FIREBALL:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                DoCast(target, SPELL_FIREBALL);
                            _events.RescheduleEvent(EVENT_FIREBALL, 5000);
                            break;
                        case EVENT_TACTICAL_BLINK:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                                DoCast(target, SPELL_TACTICAL_BLINK);
                            DoCast(me, SPELL_HELLFIRE);
                            _events.RescheduleEvent(EVENT_TACTICAL_BLINK, 12000);
                            break;
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_ymirjar_flamebearerAI(creature);
        }
};

class mob_iceborn_protodrake : public CreatureScript
{
    public:
        mob_iceborn_protodrake() : CreatureScript("mob_iceborn_protodrake") { }

        struct mob_iceborn_protodrakeAI: public ScriptedAI
        {
            mob_iceborn_protodrakeAI(Creature* creature) : ScriptedAI(creature), _vehicle(creature->GetVehicleKit())
            {
                ASSERT(_vehicle);
            }

            void Reset()
            {
                _frostBreathCooldown = 5000;
            }

            void EnterCombat(Unit* /*who*/)
            {
                _vehicle->RemoveAllPassengers();
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (_frostBreathCooldown < diff)
                {
                    DoCastVictim(SPELL_FROST_BREATH);
                    _frostBreathCooldown = 10000;
                }
                else
                    _frostBreathCooldown -= diff;

                DoMeleeAttackIfReady();
            }

        private:
            Vehicle* _vehicle;
            uint32 _frostBreathCooldown;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_iceborn_protodrakeAI(creature);
        }
};

class mob_wrathbone_laborer : public CreatureScript
{
    public:
        mob_wrathbone_laborer() : CreatureScript("mob_wrathbone_laborer") { }

        struct mob_wrathbone_laborerAI: public ScriptedAI
        {
            mob_wrathbone_laborerAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void Reset()
            {
                _events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                _events.ScheduleEvent(EVENT_BLINDING_DIRT, 8000);
                _events.ScheduleEvent(EVENT_PUNCTURE_WOUND, 9000);
                _events.ScheduleEvent(EVENT_SHOVELLED, 5000);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BLINDING_DIRT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f, true))
                                DoCast(target, SPELL_BLINDING_DIRT);
                            _events.RescheduleEvent(EVENT_BLINDING_DIRT, 10000);
                            return;
                        case EVENT_PUNCTURE_WOUND:
                            DoCastVictim(SPELL_PUNCTURE_WOUND);
                            _events.RescheduleEvent(EVENT_PUNCTURE_WOUND, 9000);
                            return;
                        case EVENT_SHOVELLED:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, -5.0f))
                                DoCast(target, SPELL_SHOVELLED);
                            _events.RescheduleEvent(EVENT_SHOVELLED, 7000);
                            return;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap _events;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_wrathbone_laborerAI(creature);
        }
};

class mob_geist_ambusher : public CreatureScript
{
    public:
        mob_geist_ambusher() : CreatureScript("mob_geist_ambusher") { }

        struct mob_geist_ambusherAI: public ScriptedAI
        {
            mob_geist_ambusherAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void Reset()
            {
                _leapingFaceMaulCooldown = 9000;
            }

            void EnterCombat(Unit* who)
            {
                if (who->GetTypeId() != TYPEID_PLAYER)
                    return;

                // the max range is determined by aggro range
                if (me->GetDistance(who) > 5.0f)
                    DoCast(who, SPELL_LEAPING_FACE_MAUL);
            }

            void UpdateAI(const uint32 diff)
            {
                if (!UpdateVictim())
                    return;

                if (_leapingFaceMaulCooldown < diff)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 5.0f, true))
                        DoCast(target, SPELL_LEAPING_FACE_MAUL);
                    _leapingFaceMaulCooldown = urand(9000, 14000);
                }
                else
                    _leapingFaceMaulCooldown -= diff;

                DoMeleeAttackIfReady();
            }

        private:
            uint32 _leapingFaceMaulCooldown;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new mob_geist_ambusherAI(creature);
        }
};

class npc_invisible_stalker_all_phases_pos : public CreatureScript
{
    public:
        npc_invisible_stalker_all_phases_pos() : CreatureScript("npc_invisible_stalker_all_phases_pos") { }

        struct npc_invisible_stalker_all_phases_posAI: public ScriptedAI
        {
            npc_invisible_stalker_all_phases_posAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void DoAction(int32 const action)
            {
                if (action != ACTION_COLLAPSE_ICICLE)
                    return;

                me->CastSpell(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 0.01f, SPELL_SUMMON_ICICLE, true);
            }

            void JustSummoned(Creature* summon)
            {
                if (summon->GetEntry() != NPC_COLLAPSING_ICICLE)
                    return;

                // TRIGGERED_IGNORE_AURA_INTERRUPT_FLAGS | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD
                summon->CastSpell(summon, SPELL_ICICLE_VISUAL, true);
                summon->CastSpell(summon, SPELL_ICICLE_DAMAGE_TRIGGER, true);
                if (summon->IsAIEnabled)
                    summon->AI()->SetGUID(me->GetGUID());
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_invisible_stalker_all_phases_posAI(creature);
        }
};

class npc_collapsing_icicle : public CreatureScript
{
    public:
        npc_collapsing_icicle() : CreatureScript("npc_collapsing_icicle") { }

        struct npc_collapsing_icicleAI: public ScriptedAI
        {
            npc_collapsing_icicleAI(Creature* creature) : ScriptedAI(creature),
                _summonerGUID(0)
            {
            }

            uint64 GetGuid(int32 /*id*/)
            {
                return _summonerGUID;
            }

            void SetGUID(uint64 guid, int32 /*type*/)
            {
                _summonerGUID = guid;
            }

        private:
            uint64 _summonerGUID;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_collapsing_icicleAI(creature);
        }
};


class spell_trash_mob_glacial_strike : public SpellScriptLoader
{
    public:
        spell_trash_mob_glacial_strike() : SpellScriptLoader("spell_trash_mob_glacial_strike") { }

        class spell_trash_mob_glacial_strike_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_trash_mob_glacial_strike_AuraScript);

            void PeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if (GetTarget()->IsFullHealth())
                {
                    GetTarget()->RemoveAura(GetId(), 0, 0, AURA_REMOVE_BY_ENEMY_SPELL);
                    PreventDefaultAction();
                }
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_trash_mob_glacial_strike_AuraScript::PeriodicTick, EFFECT_2, SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_trash_mob_glacial_strike_AuraScript();
        }
};

class spell_icicle_damage_trigger : public SpellScriptLoader
{
    public:
        spell_icicle_damage_trigger() : SpellScriptLoader("spell_icicle_damage_trigger") { }

        class spell_icicle_damage_trigger_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_icicle_damage_trigger_SpellScript);

            void HandleDummy(SpellEffIndex effIndex)
            {
                Unit* hitUnit = GetHitUnit();
                if (hitUnit->IsAIEnabled)
                    if (Creature* stalker = ObjectAccessor::GetCreature(*hitUnit, hitUnit->GetAI()->GetGUID()))
                        stalker->DespawnOrUnsummon();
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_icicle_damage_trigger_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_icicle_damage_trigger_SpellScript();
        }
};

class StalkersSearcher
{
    public:
        StalkersSearcher(Position pos, float maxRange) : _sourcePos(pos), _range(maxRange) {}
        bool operator() (Unit* unit)
        {
            return (unit->GetEntry() == NPC_INVISIBLE_STALKER_ALL_PHASES && unit->isAlive() && unit->IsInDist(&_sourcePos, _range));
        }

    private:
        const Position _sourcePos;
        float _range;
};

class AreaTrigger_at_icicle_trigger : public AreaTriggerScript
{
    public:
        AreaTrigger_at_icicle_trigger() : AreaTriggerScript("AreaTrigger_at_icicle_trigger")
        {
        }

        bool OnTrigger(Player* player, AreaTriggerEntry const* trigger)
        {
            std::list<Creature*> stalkers;
            Position triggerPos = { trigger->x, trigger->y, trigger->z, 0.f };

            StalkersSearcher check = StalkersSearcher(triggerPos, trigger->radius);
            Trinity::CreatureListSearcher<StalkersSearcher> searcher(player, stalkers, check);
            // Search around the player in the trigger's diameter range since the player may not be at the exact trigger spot.
            player->VisitNearbyGridObject(float(trigger->radius * 2), searcher);

            if (Creature* stalker = Trinity::Containers::SelectRandomContainerElement(stalkers))
            {
                stalkers.remove(stalker);
                if (stalker->IsAIEnabled)
                    stalker->GetAI()->DoAction(ACTION_COLLAPSE_ICICLE);
            }

            for (std::list<Creature*>::const_iterator itr = stalkers.begin(); itr != stalkers.end(); ++itr)
                (*itr)->DespawnOrUnsummon();
            return false;
        }
};

void AddSC_pit_of_saron()
{
    new mob_ymirjar_flamebearer();
    new mob_wrathbone_laborer();
    new mob_iceborn_protodrake();
    new mob_geist_ambusher();
    new npc_invisible_stalker_all_phases_pos();
    new npc_collapsing_icicle();
    new spell_trash_mob_glacial_strike();
    new spell_icicle_damage_trigger();
    new AreaTrigger_at_icicle_trigger();
}
