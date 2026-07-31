import logging
import argparse
import time

from pathlib import Path
from ipc import GameIPC
from commands import CommandManager
from agent import Agent
from dotenv import load_dotenv

logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
logger = logging.getLogger(__name__)

SCREENSHOTS_DIR = Path(__file__).parent / "screenshots"

def dispatch_command(cmds: CommandManager, args: list[str]) -> bool:
    command = args[0]
    action = args[1] if len(args) > 1 else None

    if command == "navigate":
        if action in ("walk", "run"):
            cmds.navigate(action, int(args[2]), int(args[3]))
        elif action in ("scroll_left", "scroll_right", "scroll_up", "scroll_down", "center"):
            cmds.navigate(action)
        else:
            logger.error("navigate: unknown action '%s'", action)
            return False
    elif command == "combat":
        if action in ("begin", "end_turn", "end_combat"):
            cmds.combat(action)
        elif action == "attack":
            cmds.combat(action, int(args[2]), int(args[3]))
        else:
            logger.error("combat: unknown action '%s'", action)
            return False
    elif command == "inventory":
        if action == "open":
            cmds.inventory("open")
        elif action == "equip":
            cmds.inventory("equip", item=int(args[2]), slot=int(args[3]))
        elif action == "unequip":
            cmds.inventory("unequip", slot=int(args[2]))
        else:
            logger.error("inventory: unknown action '%s'", action)
            return False
    elif command == "loot":
        if action in ("take", "put"):
            cmds.loot(action == "take", int(args[2]), int(args[3]))
        else:
            logger.error("loot: unknown action '%s'", action)
            return False
    elif command == "barter":
        if action in ("buy", "sell", "retract_mine", "retract_theirs", "confirm"):
            cmds.barter(action, int(args[2]) if len(args) > 2 else 0, int(args[3]) if len(args) > 3 else 0)
        else:
            logger.error("barter: unknown action '%s'", action)
            return False
    elif command == "mouse":
        if action in ("click", "context_menu"):
            cmds.mouse(action, int(args[2]), int(args[3]))
        elif action == "context_select":
            cmds.mouse(action, int(args[2]))
        else:
            logger.error("mouse: unknown action '%s'", action)
            return False
    elif command == "save":
        if action in ("save", "load", "quick_save", "quick_load"):
            cmds.save(action)
        else:
            logger.error("save: unknown action '%s'", action)
            return False
    elif command == "item":
        if action in ("change_hand", "change_mode"):
            cmds.item(action)
        else:
            logger.error("item: unknown action '%s'", action)
            return False
    elif command == "text_input":
        cmds.text_input(action)
    else:
        logger.error("Unknown command: '%s'", command)
        return False

    return True

def print_response(response):
    for msg in response.messages:
        logger.info("[Response] %s", msg)
    if response.state:
        s = response.state
        logger.info("[State] HP: %d/%d AP: %d/%d AC: %d Level: %d",
                    s.hp, s.max_hp, s.ap, s.max_ap, s.ac, s.level)
    if response.screenshot:
        SCREENSHOTS_DIR.mkdir(exist_ok=True)
        path = SCREENSHOTS_DIR / "latest.png"
        response.screenshot.save(path)
        logger.info("[Screenshot] Saved to %s", path)

def main():
    parser = argparse.ArgumentParser(description="Claude Plays Fallout")
    parser.add_argument(
        "--manual",
        action="store_true",
        default=False,
        help="Run manual command testing loop"
    )
    parser.add_argument(
        "--max-history",
        type=int,
        default=30,
        help="Maximum number of messages in history before summarization"
    )
    parser.add_argument(
        "--host",
        type=str,
        default="127.0.0.1",
        help="Host address"
    )
    parser.add_argument(
        "--port",
        type=int,
        default=49152,
        help="TCP port to connect back to the game"
    )
    parser.add_argument(
        "--steps", 
        type=int, 
        default=10, 
        help="Number of agent steps to run"
    )
    args = parser.parse_args()

    ipc = GameIPC()
    ipc.connect(args.host, args.port)
    cmds = CommandManager(ipc)

    load_dotenv()

    if args.manual:
        try:
            while True:
                cmd = input("> ").strip()
                if not cmd:
                    continue

                cmds.request_state()
                response = cmds.recv_response()
                print_response(response)

                cmd_args = cmd.split()
                if not dispatch_command(cmds, cmd_args):
                    continue
        except (EOFError, KeyboardInterrupt):
            pass
    else:
        agent = Agent(logger, cmds, max_history=args.max_history)
        time.sleep(2)
        agent.run(args.steps)

if __name__ == "__main__":
    main()
