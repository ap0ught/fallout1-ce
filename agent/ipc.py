import logging
import socket
import struct

from enum import Enum

logger = logging.getLogger(__name__)

class MessageType(Enum):
    MSG_GENERAL = 0x00
    MSG_SCREENSHOT = 0x01
    MSG_PLAYER_STATE = 0x02
    MSG_RESPONSE_END = 0x03

    MSG_NAVIGATE = 0x10
    MSG_COMBAT = 0x11
    MSG_INVENTORY = 0x12
    MSG_BARTER = 0x13
    MSG_ITEM = 0x14
    MSG_LOOT = 0x15
    MSG_REQ_STATE = 0x16
    MSG_SAVE = 0x17
    MSG_MOUSE = 0x18
    MSG_TEXT_INPUT = 0x19

class GameIPC:
    def __init__(self):
        self._sock = None

    def connect(self, host: str, port: int):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.connect((host, port))
        logger.info("Connected to game on port %d", port)

    def send(self, msg_type: MessageType, payload: bytes = b""):
        header = struct.pack("!BI", msg_type.value, len(payload))
        self._sock.sendall(header + payload)

    def recv_response_raw(self) -> list[tuple[MessageType, bytes]]:
        response = []
        while True:
            msg_type, payload = self._recv_msg()
            if msg_type == MessageType.MSG_RESPONSE_END:
                return response
            response.append((msg_type, payload))

    def close(self):
        if self._sock:
            self._sock.close()
            self._sock = None

    def _recv_exact(self, n: int) -> bytes:
        buf = bytearray()
        while len(buf) < n:
            chunk = self._sock.recv(min(n - len(buf), 4096))
            if not chunk:
                raise ConnectionError("Socket closed while reading")
            buf.extend(chunk)
        return bytes(buf)

    def _recv_msg(self) -> tuple[MessageType, bytes]:
        header = self._recv_exact(5)
        msg_type = MessageType(header[0])
        length = struct.unpack('!I', header[1:5])[0]
        payload = self._recv_exact(length) if length > 0 else b""
        return (msg_type, payload)
