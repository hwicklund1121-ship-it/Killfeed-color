import asyncio
import json
from pathlib import Path
import sys
import tempfile
from unittest import IsolatedAsyncioTestCase, TestCase
from unittest.mock import AsyncMock, Mock, patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import discord
from bot import LoggingBot, embed_for

class FormattingTests(TestCase):
    def test_untrusted_text_and_limits(self):
        embed = embed_for({'Id': 'test', 'Kind': 'kill', 'PlayerName': '@everyone **admin**', 'Details': '@here ' + 'x' * 10000})
        self.assertLess(len(embed), 6000)
        self.assertNotIn('@everyone', embed.fields[0].value)
        self.assertNotIn('@here', embed.description)
        self.assertIn('\\*', embed.fields[0].value)

class AlertFormattingTests(TestCase):
    def test_review_flag_is_visible_and_routes_to_staff_channel(self):
        from engine import routes
        event = dict(Schema=1, Id='review-1', Kind='gameplay_speed', PlayerName='Player',
                     Details='REVIEW FLAG; not proof of cheating. Horizontal speed 25 m/s')
        self.assertEqual(routes(event), ('anticheat-logs',))
        embed = embed_for(event)
        self.assertEqual(embed.colour.value, 0xE6A23C)
        self.assertIn('not proof of cheating', embed.description)

class DeliveryTests(IsolatedAsyncioTestCase):
    async def test_mocked_send_and_failed_channel_retry(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'events-boot.jsonl').write_text(json.dumps(dict(Schema=1, Id='boot-1', Kind='duel_invite')) + '\n')
            bot = LoggingBot(123, root, {}, root / 'queue.db')
            good = Mock(send=AsyncMock())
            response = Mock(status=403, reason='Forbidden')
            bad = Mock(send=AsyncMock(side_effect=discord.Forbidden(response, 'missing permissions')))
            bot.channels = Mock(return_value={'duel-logs': bad, 'duel-invites': good})
            bot.wait_until_ready = AsyncMock()
            with patch('bot.asyncio.sleep', new=AsyncMock(side_effect=asyncio.CancelledError)):
                with self.assertRaises(asyncio.CancelledError):
                    await bot.work()
            good.send.assert_awaited_once()
            self.assertFalse(good.send.call_args.kwargs['allowed_mentions'].everyone)
            self.assertEqual(bot.store.stats(), {'pending': 1, 'delivered': 1, 'rejected': 0})
            self.assertEqual(bot.store.db.execute('SELECT error FROM outbox WHERE delivered IS NULL').fetchone()[0], 'Discord HTTP 403')
            await bot.close()

    async def test_gameplay_event_reaches_the_discord_channel(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            event = dict(Schema=1, Id='review-1', Kind='gameplay_fire_rate', PlayerName='Player',
                         Details='REVIEW FLAG; not proof of cheating. Shots 6 in 2500 ms')
            (root / 'events-boot.jsonl').write_text(json.dumps(event) + '\n')
            bot = LoggingBot(123, root, {}, root / 'queue.db')
            channel = Mock(send=AsyncMock())
            bot.channels = Mock(return_value={'anticheat-logs': channel})
            bot.wait_until_ready = AsyncMock()
            with patch('bot.asyncio.sleep', new=AsyncMock(side_effect=asyncio.CancelledError)):
                with self.assertRaises(asyncio.CancelledError):
                    await bot.work()
            channel.send.assert_awaited_once()
            sent = channel.send.call_args.kwargs['embed']
            self.assertIn('Shots 6 in 2500 ms', sent.description)
            self.assertEqual(bot.store.stats()['pending'], 0)
            self.assertEqual(bot.store.stats()['delivered'], 1)
            await bot.close()
