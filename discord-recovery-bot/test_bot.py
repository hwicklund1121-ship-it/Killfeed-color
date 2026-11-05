"""Offline checks: no real token or Discord connection."""
import asyncio
import os
import time
import unittest
from datetime import datetime, timezone, timedelta
from types import SimpleNamespace as NS
from unittest.mock import AsyncMock, patch
os.environ['SERVER_ID']='123456789012345678'
os.environ['DISCORD_BOT_TOKEN']='not-a-real-token'
os.environ['TRUSTED_USER_IDS']='77'
os.environ['AUTO_KICK']='true'
import discord
import bot as module
from protection import Protection

class PolicyTests(unittest.TestCase):
    def test_threshold_is_per_actor_and_action(self):
        p=Protection()
        for eid, actor, action in [(1,11,'channel_delete'),(2,22,'channel_delete'),(3,11,'channel_create'),(4,11,'channel_delete')]:
            self.assertFalse(p.observe(eid,actor,action,eid))
        self.assertTrue(p.observe(5,11,'channel_delete',5))
    def test_duplicates_do_not_count_twice(self):
        p=Protection()
        self.assertFalse(p.observe(1,11,'channel_delete',0))
        self.assertFalse(p.observe(1,11,'channel_delete',1))
        self.assertFalse(p.observe(2,11,'channel_delete',2))
        self.assertTrue(p.observe(3,11,'channel_delete',3))
    def test_expiry(self):
        p=Protection()
        p.observe(1,11,'channel_delete',0)
        p.observe(2,11,'channel_delete',1)
        self.assertFalse(p.observe(3,11,'channel_delete',12))
    def test_event_history_is_bounded_during_burst(self):
        p=Protection()
        for eid in range(100):
            p.observe(eid,11,'channel_delete',1)
        self.assertEqual(len(p.events[(11,'channel_delete')]),3)
    def test_exemptions(self):
        p=Protection()
        for i in range(20):
            self.assertFalse(p.observe(i,11,'role_delete',i,exempt=True))
        self.assertFalse(p.events)

class BotTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.bot=module.bot
        self.bot._connection.user=NS(id=99)
        self.bot.policy=Protection()
        self.bot.interventions={}
        self.bot.pending.clear()
        self.bot.cleanup_lock=asyncio.Lock()
    def guild(self, top=1):
        self.actor=NS(top_role=top,kick=AsyncMock())
        return NS(id=module.GUILD_ID,owner_id=1,me=NS(top_role=10,guild_permissions=NS(kick_members=True)),fetch_member=AsyncMock(return_value=self.actor))
    def entry(self,guild,actor=5,eid=1,age=0):
        return NS(guild=guild,user_id=actor,id=eid,action=discord.AuditLogAction.role_delete,created_at=datetime.now(timezone.utc)-timedelta(seconds=age))
    async def test_kick_once(self):
        guild=self.guild()
        await self.bot.on_audit_log_entry_create(self.entry(guild))
        await self.bot.on_audit_log_entry_create(self.entry(guild,eid=2))
        self.actor.kick.assert_awaited_once()
    async def test_owner_bot_and_trusted_exempt(self):
        guild=self.guild()
        for i,actor in enumerate([1,99,77]):
            await self.bot.on_audit_log_entry_create(self.entry(guild,actor,i))
        guild.fetch_member.assert_not_awaited()
    async def test_hierarchy(self):
        guild=self.guild(top=10)
        await self.bot.on_audit_log_entry_create(self.entry(guild))
        self.actor.kick.assert_not_awaited()
    async def test_dangerous_permission_grant(self):
        guild=self.guild()
        entry=self.entry(guild)
        entry.action=discord.AuditLogAction.role_update
        entry.before=NS(permissions=discord.Permissions.none())
        entry.after=NS(permissions=discord.Permissions(administrator=True))
        await self.bot.on_audit_log_entry_create(entry)
        self.actor.kick.assert_awaited_once()
    async def test_channel_overwrites_count_together(self):
        guild=self.guild()
        for i in range(6):
            entry=self.entry(guild,eid=i)
            entry.action=discord.AuditLogAction.overwrite_update
            await self.bot.on_audit_log_entry_create(entry)
        self.actor.kick.assert_awaited_once()
    async def test_stale_event_ignored(self):
        guild=self.guild()
        await self.bot.on_audit_log_entry_create(self.entry(guild,age=40))
        guild.fetch_member.assert_not_awaited()
    async def test_wrong_confirmation_keeps_preview(self):
        interaction=NS(user=NS(id=1),response=NS(send_message=AsyncMock()))
        self.bot.pending[1]=module.Preview('correct',time.monotonic()+60,(1,))
        await module.cleanup_confirm.callback(interaction,str(module.GUILD_ID),'wrong')
        interaction.response.send_message.assert_awaited_once()
        self.assertIn(1,self.bot.pending)
    async def test_snapshot_only_and_caller_last(self):
        order=[]
        def channel(cid):
            async def delete(**kwargs):order.append(cid)
            return NS(id=cid,name=str(cid),delete=delete)
        guild=NS(fetch_channels=AsyncMock(return_value=[channel(10),channel(20),channel(30)]))
        interaction=NS(user=NS(id=1),guild=guild,channel_id=10,channel=None,response=NS(defer=AsyncMock()),edit_original_response=AsyncMock())
        self.bot.pending[1]=module.Preview('correct',time.monotonic()+60,(10,20))
        with patch.object(module.asyncio,'sleep',new=AsyncMock()):
            await module.cleanup_confirm.callback(interaction,str(module.GUILD_ID),'correct')
        self.assertEqual(order,[20,10])
        self.assertNotIn(1,self.bot.pending)
    def test_owner_check(self):
        guild=NS(id=module.GUILD_ID,owner_id=1)
        self.assertTrue(module.owner_only(NS(guild=guild,user=NS(id=1))))
        self.assertFalse(module.owner_only(NS(guild=guild,user=NS(id=2))))
        self.assertFalse(module.owner_only(NS(guild=NS(id=4,owner_id=1),user=NS(id=1))))
if __name__=='__main__':unittest.main()
