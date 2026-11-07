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
