"""Rebuild regression tests: no network or live Discord changes."""
import asyncio
import logging
import tempfile
import unittest
from datetime import datetime, timezone
from types import SimpleNamespace as NS
from unittest.mock import AsyncMock, patch

import discord
from recovery import PlanStore, Rebuilder, collect_deleted, deleted_record


class Target:
    def __init__(self, identifier): self.id = identifier


class FakeChannel:
    def __init__(self, guild, identifier, name, category=None):
        self.guild, self.id, self.name, self.category = guild, identifier, name, category
    async def delete(self, **kwargs):
        self.guild.events.append(('delete', self.id))
        self.guild.live.pop(self.id, None)


class FakeCategory(FakeChannel): pass


class FakeGuild:
    id = 123
    owner_id = 7
    features = []
    bitrate_limit = 96000
    rules_channel = None
    public_updates_channel = None
    def __init__(self):
        self.default_role = Target(self.id)
        self.me = Target(99)
        self.me.guild_permissions = NS(manage_channels=True, view_audit_log=True, view_channel=True, send_messages=True, attach_files=True, administrator=False, value=discord.Permissions.all().value)
        self._state = NS()
        self.live = {1:FakeChannel(self,1,'spam-one'), 2:FakeChannel(self,2,'spam-two')}
        self.events = []
        self.rows = []
        self.last_id = 1000
        self.created = []
    async def fetch_channels(self): return list(self.live.values())
    def get_channel(self, identifier): return self.live.get(identifier)
    def get_role(self, identifier): return self.default_role if identifier == self.id else None
    def get_member(self, identifier): return self.me if identifier == self.me.id else None
    async def fetch_member(self, identifier): raise AssertionError('Unexpected member lookup')
    async def audit_logs(self, **kwargs):
        for row in self.rows:
            yield row
    async def create_text_channel(self, name, **kwargs): return self.create(name,False,kwargs)
    async def create_category(self, name, **kwargs): return self.create(name,True,kwargs)
    async def create_voice_channel(self, name, **kwargs): return self.create(name,False,kwargs)
    async def create_forum(self, name, **kwargs): return self.create(name,False,kwargs)
    async def create_stage_channel(self, name, **kwargs): return self.create(name,False,kwargs)
    def create(self,name,category,kwargs):
        self.last_id += 1
        c=(FakeCategory if category else FakeChannel)(self,self.last_id,name,kwargs.get('category'))
        self.live[c.id]=c
        self.events.append(('create',name))
        self.created.append((c,kwargs))
        return c


def old(identifier,name,kind=0,parent=None,known=False):
    return dict(old_id=identifier,name=name,kind=kind,parent_id=parent,position=0,permissions_known=known,overwrites=[])


def plan():
    return dict(code='abcdef123456',guild_id=123,owner_id=7,approved=True,phase='prepared',
                created_at=datetime.now(timezone.utc).isoformat(),current_ids=[1,2],current=[],
                old=[old(10,'original-category',4),old(20,'original-chat',parent=10)],restored={},notes=[])


class RecoveryTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.store = PlanStore(self.tmp.name)
        self.guild = FakeGuild()
        self.rebuilder = Rebuilder(self.store,logging.getLogger('test'))

    async def test_no_deletion_when_no_recoverable_channels(self):
        p=plan(); p['old']=[]
        with self.assertRaises(ValueError): await self.rebuilder.run(self.guild,p)
        self.assertFalse(self.guild.events)

    async def test_unapproved_plan_is_rejected(self):
        p=plan(); p['approved']=False
        with self.assertRaises(ValueError): await self.rebuilder.run(self.guild,p)
        self.assertFalse(self.guild.events)

    async def test_delete_current_then_restore_with_category_and_no_role_api(self):
        p=plan()
        with patch('recovery.discord.CategoryChannel',FakeCategory), patch('recovery.asyncio.sleep',new=AsyncMock()):
            count,failed=await self.rebuilder.run(self.guild,p)
        self.assertEqual((count,failed),(2,[]))
        events=self.guild.events
        self.assertLess(events.index(('delete',2)),events.index(('create','original-category')))
        self.assertLess(events.index(('create','original-category')),events.index(('create','original-chat')))
        chat=next((c,k) for c,k in self.guild.created if c.name=='original-chat')
        self.assertEqual(chat[0].category.name,'original-category')
        self.assertFalse(chat[1]['overwrites'][self.guild.default_role].view_channel)
        self.assertEqual(self.store.load(p['code'])['phase'],'complete')

    async def test_resume_never_deletes_new_unreviewed_channel_or_duplicates_old(self):
        p=plan()
        with patch('recovery.discord.CategoryChannel',FakeCategory), patch('recovery.asyncio.sleep',new=AsyncMock()):
            await self.rebuilder.run(self.guild,p)
            self.guild.live[555]=FakeChannel(self.guild,555,'new-unreviewed')
            await self.rebuilder.run(self.guild,p)
        self.assertIn(555,self.guild.live)
        self.assertEqual(self.guild.events.count(('create','original-chat')),1)
        self.assertEqual(self.guild.events.count(('create','original-category')),1)

    async def test_failed_delete_stops_before_recreating_old_channels(self):
        p=plan()
        async def blocked(**kwargs):
            raise discord.Forbidden(NS(status=403,reason='Forbidden'),{'message':'Missing permissions','code':50013})
        self.guild.live[1].delete=blocked
        with patch('recovery.asyncio.sleep',new=AsyncMock()):
            with self.assertRaises(ValueError): await self.rebuilder.run(self.guild,p)
        self.assertIn(2,self.guild.live)
        self.assertNotIn(('create','original-category'),self.guild.events)
        self.assertEqual(self.store.load(p['code'])['phase'],'deleting')

    async def test_interrupted_journal_reconciles_created_channel_audit(self):
        p=plan(); p['phase']='restoring'; p['old']=[old(20,'original-chat')]
        self.guild.live[555]=FakeChannel(self.guild,555,'original-chat')
        self.guild.rows=[NS(reason='GRZ rebuild abcdef123456 source=20',user_id=99,target=Target(555))]
        with patch('recovery.asyncio.sleep',new=AsyncMock()):
            await self.rebuilder.run(self.guild,p)
        self.assertEqual(p['restored']['20'],555)
        self.assertNotIn(('create','original-chat'),self.guild.events)

    async def test_missing_permission_stops_before_deletion(self):
        self.guild.me.guild_permissions.manage_channels=False
        with self.assertRaises(ValueError): await self.rebuilder.run(self.guild,plan())
        self.assertFalse(self.guild.events)

    async def test_required_community_channel_stops_before_deletion(self):
        self.guild.features=['COMMUNITY']; self.guild.rules_channel=self.guild.live[1]
        with self.assertRaises(ValueError): await self.rebuilder.run(self.guild,plan())
        self.assertFalse(self.guild.events)

    async def test_audit_scan_skips_bot_cleanup_and_requires_records(self):
        now=datetime.now(timezone.utc)
        self.guild.rows=[NS(before=NS(name='old-chat',type=discord.ChannelType.text,parent_id='88'),
                            target=Target(55),guild=self.guild,user_id=5,created_at=now)]
        records,skipped=await collect_deleted(self.guild,24,None,99)
        self.assertEqual(records[0]['parent_id'],88)
        self.assertFalse(records[0]['permissions_known'])
        self.guild.rows[0].user_id=99
        with self.assertRaises(ValueError): await collect_deleted(self.guild,24,None,99)
        explicit,_=await collect_deleted(self.guild,24,99,99)
        self.assertEqual(len(explicit),1)

    async def test_missing_role_permissions_fall_back_to_private(self):
        record=old(20,'private',known=True)
        record['overwrites']=[dict(id=888,kind='role',allow=1024,deny=0)]
        overwrites,private=await self.rebuilder.overwrites(self.guild,record)
        self.assertTrue(private)
        self.assertFalse(overwrites[self.guild.default_role].view_channel)

    def test_raw_discord_audit_payload_is_recoverable(self):
        entry=discord.AuditLogEntry(users={},integrations={},app_commands={},automod_rules={},webhooks={},guild=self.guild,
            data=dict(id=str(discord.utils.time_snowflake(datetime.now(timezone.utc))),action_type=12,target_id='55',user_id='5',changes=[
                dict(key='name',old_value='original-chat'),dict(key='type',old_value=0),
                dict(key='parent_id',old_value='66'),dict(key='position',old_value=3),
                dict(key='permission_overwrites',old_value=[dict(id='123',type='0',allow='1024',deny='0')])]))
        record=deleted_record(entry)
        self.assertEqual(record['name'],'original-chat')
        self.assertEqual(record['parent_id'],66)
        self.assertEqual(record['kind'],0)
        self.assertTrue(record['permissions_known'])
        self.assertEqual(record['overwrites'],[dict(id=123,kind='role',allow=1024,deny=0)])

    async def test_ungrantable_permissions_use_private_fallback(self):
        record=old(20,'old',known=True)
        record['overwrites']=[dict(id=123,kind='role',allow=8,deny=0)]
        self.guild.me.guild_permissions.value=1024|2048|32768
        overwrites,private=await self.rebuilder.overwrites(self.guild,record)
        self.assertTrue(private)
        self.assertFalse(overwrites[self.guild.default_role].view_channel)

    def test_plan_store_rejects_path_traversal(self):
        with self.assertRaises(ValueError): self.store.load('../../secrets')


if __name__=='__main__': unittest.main()
