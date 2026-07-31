import logging
import struct

from dataclasses import dataclass
from ipc import GameIPC, MessageType
from PIL import Image

logger = logging.getLogger(__name__)

@dataclass
class GameState:
    hp: int
    max_hp: int
    ap: int
    max_ap: int
    ac: int
    level: int

@dataclass
class GameResponse:
    messages: list[str]
    screenshot: Image.Image | None
    state: GameState | None

class CommandManager:
    def __init__(self, ipc: GameIPC):
        self._ipc = ipc

    def navigate(self, action: str, x: int = 0, y: int = 0):
        action_map = {
            "walk": 0, "run": 1, "scroll_left": 2, "scroll_right": 3,
            "scroll_up": 4, "scroll_down": 5, "center": 6
        }
        code = action_map[action]
        if action in ("walk", "run"):
            self._ipc.send(MessageType.MSG_NAVIGATE, struct.pack("!BII", code, x, y))
        else:
            self._ipc.send(MessageType.MSG_NAVIGATE, struct.pack("!B", code))

    def combat(self, action: str, x: int = 0, y: int = 0):
        action_map = {
            "begin": 0, "end_turn": 1, "end_combat": 2, "attack": 3
        }
        code = action_map[action]
        if action == "attack":
            self._ipc.send(MessageType.MSG_COMBAT, struct.pack("!BII", code, x, y))
        else:
            self._ipc.send(MessageType.MSG_COMBAT, struct.pack("!B", code))

    def inventory(self, action: str, item: int = 0, slot: int = 0):
        action_map = {"open": 0, "equip": 1, "unequip": 2}
        code = action_map[action]
        if action == "equip":
            self._ipc.send(MessageType.MSG_INVENTORY, struct.pack("!BIB", code, item, slot))
        elif action == "unequip":
            self._ipc.send(MessageType.MSG_INVENTORY, struct.pack("!BB", code, slot))
        else:
            self._ipc.send(MessageType.MSG_INVENTORY, struct.pack("!B", code))

    def loot(self, take: bool, item: int, quantity: int):
        self._ipc.send(MessageType.MSG_LOOT, struct.pack("!BBI", int(take), item, quantity))

    def barter(self, action: str, item: int = 0, quantity: int = 0):
        action_map = {
            "buy": 0, "sell": 1, "retract_mine": 2, "retract_theirs": 3, "confirm": 4
        }
        code = action_map[action]
        self._ipc.send(MessageType.MSG_BARTER, struct.pack("!BII", code, item, quantity))

    def mouse(self, action: str, x: int = 0, y: int = 0):
        action_map = {"click": 0, "context_menu": 1, "context_select": 2}
        code = action_map[action]
        self._ipc.send(MessageType.MSG_MOUSE, struct.pack("!BII", code, x, y))

    def item(self, action: str):
        action_map = {"change_hand": 0, "change_mode": 1}
        code = action_map[action]
        self._ipc.send(MessageType.MSG_ITEM, struct.pack("!B", code))

    def save(self, action: str):
        action_map = {"save": 0, "load": 1, "quick_save": 2, "quick_load": 3}
        code = action_map[action]
        self._ipc.send(MessageType.MSG_SAVE, struct.pack("!B", code))

    def text_input(self, text: str):
        encoded = text.encode("utf-8")
        self._ipc.send(MessageType.MSG_TEXT_INPUT, struct.pack("!I", len(encoded)) + encoded + b"\x00")

    def request_state(self):
        self._ipc.send(MessageType.MSG_REQ_STATE)

    def recv_response(self) -> GameResponse:
        raw_messages = self._ipc.recv_response_raw()

        messages = []
        screenshot = None
        state = None

        for msg_type, payload in raw_messages:
            if msg_type == MessageType.MSG_GENERAL:
                messages.append(payload.decode("utf-8"))
            elif msg_type == MessageType.MSG_SCREENSHOT:
                w, h = struct.unpack('!II', payload[:8])
                pixel_data = payload[8:]
                screenshot = Image.frombytes("RGB", (w, h), pixel_data, "raw", "BGRX")
            elif msg_type == MessageType.MSG_PLAYER_STATE:
                hp, max_hp, ap, max_ap, ac, level = struct.unpack('!iiiiii', payload[:24])
                state = GameState(hp=hp, max_hp=max_hp, ap=ap, max_ap=max_ap, ac=ac, level=level)

        return GameResponse(messages=messages, screenshot=screenshot, state=state)

