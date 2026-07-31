import base64
import copy
import io
import time
import os

from config import MAX_TOKENS, MODEL_NAME, TEMPERATURE
from commands import CommandManager, GameResponse
from anthropic import Anthropic

SYSTEM_PROMPT = """You are playing Fallout 1. You can see the game screen and control the game using categorized commands/tools.

Before each action, explain your reasoning briefly, then use the tools to execute your chosen commands. 

When clicking an object/menu item, aim for the center. When in menus, aim for the center of the RED button.

The conversation history may occasionally be summarized to save context space. If you see a message labeled "CONVERSATION HISTORY SUMMARY", this contains the key information about your progress so far."""

SUMMARY_PROMPT = """I need you to create a detailed summary of our conversation history up to this point. This summary will replace the full conversation history to manage the context window.

Please include:
1. Key game events and milestones you've reached
2. Important decisions you've made
3. Current objectives or goals you're working toward
4. Your current location and status
5. Any strategies or plans you've mentioned

The summary should be comprehensive enough that you can continue gameplay without losing important context about what has happened so far."""

AVAILABLE_TOOLS = [
    {
        "name": "navigate",
        "description": "Control player or camera movement in the game world.",
        "input_schema": {
            "type": "object",
            "properties": {
                "action": {
                    "type": "string",
                    "enum": ["walk", "run", "scroll_left", "scroll_right", "scroll_up", "scroll_down", "center"]
                },
                "x": {"type": "integer", "description": "Target x coordinate (required for walk/run)"},
                "y": {"type": "integer", "description": "Target y coordinate (required for walk/run)"}
            },
            "required": ["action"]
        }
    },
    {
        "name": "combat",
        "description": "Execute combat-related actions.",
        "input_schema": {
            "type": "object",
            "properties": {
                "action": {
                    "type": "string",
                    "enum": ["begin", "end_turn", "end_combat", "attack"]
                },
                "x": {"type": "integer", "description": "Target x coordinate (required for attack)"},
                "y": {"type": "integer", "description": "Target y coordinate (required for attack)"}
            },
            "required": ["action"]
        }
    },
    {
        "name": "inventory",
        "description": "Manage inventory items and equipment. **Only available in the inventory menu**",
        "input_schema": {
            "type": "object",
            "properties": {
                "action": {
                    "type": "string",
                    "enum": ["open", "equip", "unequip"]
                },
                "item": {"type": "integer", "description": "Item ID (required for equip)"},
                "slot": {"type": "integer", "description": "Equipment slot index (required for equip/unequip)"}
            },
            "required": ["action"]
        }
    },
    {
        "name": "loot",
        "description": "Take or put items from a container. **Only available in the container menu.**",
        "input_schema": {
            "type": "object",
            "properties": {
                "action": {
                    "type": "string",
                    "enum": ["take", "put"]
                },
                "item": {"type": "integer"},
                "quantity": {"type": "integer"}
            },
            "required": ["action", "item", "quantity"]
        }
    },
    {
        "name": "barter",
        "description": "Trade items with NPCs. **Only available in the dialogue menu.**",
        "input_schema": {
            "type": "object",
            "properties": {
                "action": {
                    "type": "string",
                    "enum": ["buy", "sell", "retract_mine", "retract_theirs", "confirm"]
                },
                "item": {"type": "integer"},
                "quantity": {"type": "integer"}
            },
            "required": ["action"]
        }
    },
    {
        "name": "mouse",
        "description": "Simulate mouse interaction.",
        "input_schema": {
            "type": "object",
            "properties": {
                "action": {
                    "type": "string",
                    "enum": ["click", "context_menu", "context_select"]
                },
                "x": {"type": "integer"},
                "y": {"type": "integer"}
            },
            "required": ["action"]
        }
    },
    {
        "name": "item",
        "description": "Toggle active item hand or item mode.",
        "input_schema": {
            "type": "object",
            "properties": {
                "action": {
                    "type": "string",
                    "enum": ["change_hand", "change_mode"]
                }
            },
            "required": ["action"]
        }
    },
    {
        "name": "save",
        "description": "Save or load game state.",
        "input_schema": {
            "type": "object",
            "properties": {
                "action": {
                    "type": "string",
                    "enum": ["save", "load", "quick_save", "quick_load"]
                }
            },
            "required": ["action"]
        }
    },
    {
        "name": "text_input",
        "description": "Send text input to the game (character creation, saving, etc...).",
        "input_schema": {
            "type": "object",
            "properties": {
                "text": {"type": "string"}
            },
            "required": ["text"]
        }
    },
    {
        "name": "request_state",
        "description": "Request current game state, messages, and screenshot.",
        "input_schema": {
            "type": "object",
            "properties": {}
        }
    }
]

def get_screenshot_base64(screenshot, upscale=1):
    """Convert PIL image to base64 string."""
    if upscale > 1:
        new_size = (screenshot.width * upscale, screenshot.height * upscale)
        screenshot = screenshot.resize(new_size)

    buffered = io.BytesIO()
    screenshot.save(buffered, format="PNG")
    return base64.standard_b64encode(buffered.getvalue()).decode()

class Agent:
    def __init__(self, logger, cmd: CommandManager, max_history=60):
        self.client = Anthropic(api_key=os.getenv("ANTHROPIC_API_KEY"))
        self.message_history = [{
            "role": "user",
            "content": [
                {"type": "text", "text": "You may now begin playing."}
            ],
        }]
        self.max_history = max_history
        self.running = True
        self.logger = logger
        self.cmd = cmd

    def _format_state_content(self, response: GameResponse):
        content = []

        if response.messages:
            content.append({"type": "text", "text": "\n".join(response.messages)})

        if response.state:
            s = response.state
            state_text = (f"HP: {s.hp}/{s.max_hp} | AP: {s.ap}/{s.max_ap} | "
                          f"AC: {s.ac} | Level: {s.level}")
            content.append({"type": "text", "text": state_text})

        if response.screenshot:
            screenshot_b64 = get_screenshot_base64(response.screenshot, upscale=1)
            content.append({
                "type": "image",
                "source": {
                    "type": "base64",
                    "media_type": "image/png",
                    "data": screenshot_b64,
                }
            })

        if not content:
            content.append({"type": "text", "text": "No state available."})

        return content

    def process_tool_call(self, tool_call):
        tool_name = tool_call.name
        tool_input = tool_call.input
        self.logger.info(f"Processing tool call: {tool_name}")

        if tool_name == "navigate":
            self.cmd.navigate(tool_input["action"],
                              tool_input.get("x", 0), tool_input.get("y", 0))
        elif tool_name == "combat":
            self.cmd.combat(tool_input["action"],
                            tool_input.get("x", 0), tool_input.get("y", 0))
        elif tool_name == "inventory":
            self.cmd.inventory(tool_input["action"],
                               tool_input.get("item", 0), tool_input.get("slot", 0))
        elif tool_name == "loot":
            self.cmd.loot(tool_input["action"] == "take", tool_input["item"], tool_input["quantity"])
        elif tool_name == "barter":
            self.cmd.barter(tool_input["action"],
                            tool_input.get("item", 0), tool_input.get("quantity", 0))
        elif tool_name == "item":
            self.cmd.item(tool_input["action"])
        elif tool_name == "mouse":
            self.cmd.mouse(tool_input["action"],
                           tool_input.get("x", 0), tool_input.get("y", 0))
        elif tool_name == "save":
            self.cmd.save(tool_input["action"])
        elif tool_name == "text_input":
            self.cmd.text_input(tool_input["text"])
        elif tool_name == "request_state":
            self.cmd.request_state()
        else:
            self.logger.error(f"Unknown tool called: {tool_name}")
            return {
                "type": "tool_result",
                "tool_use_id": tool_call.id,
                "content": [
                    {"type": "text", "text": f"Error: Unknown tool '{tool_name}'"}
                ],
            }

        response = self.cmd.recv_response()
        return self._format_tool_result(tool_call, response)

    def _format_tool_result(self, tool_call, response: GameResponse):
        content = []

        if response.messages:
            content.append({"type": "text", "text": "\n".join(response.messages)})

        if response.state:
            s = response.state
            state_text = (f"HP: {s.hp}/{s.max_hp} | AP: {s.ap}/{s.max_ap} | "
                          f"AC: {s.ac} | Level: {s.level}")
            content.append({"type": "text", "text": state_text})

        if response.screenshot:
            screenshot_b64 = get_screenshot_base64(response.screenshot, upscale=1)
            content.append({
                "type": "image",
                "source": {
                    "type": "base64",
                    "media_type": "image/png",
                    "data": screenshot_b64,
                }
            })

        if not content:
            content.append({"type": "text", "text": "Command executed."})

        return {
            "type": "tool_result",
            "tool_use_id": tool_call.id,
            "content": content,
        }

    def _request_state(self):
        self.cmd.request_state()
        return self.cmd.recv_response()

    def run(self, num_steps=1):
        self.logger.info(f"Starting agent loop for {num_steps} steps")

        steps_completed = 0
        while self.running and steps_completed < num_steps:
            try:
                # hack to get the current state
                time.sleep(5)
                state_response = self._request_state()
                state_content = self._format_state_content(state_response)

                if self.message_history and self.message_history[-1]["role"] == "user":
                    last_content = self.message_history[-1]["content"]
                    if isinstance(last_content, list):
                        last_content.extend(state_content)
                    else:
                        self.message_history[-1]["content"] = [
                            {"type": "text", "text": str(last_content)}
                        ] + state_content
                else:
                    self.message_history.append({
                        "role": "user",
                        "content": state_content,
                    })

                messages = copy.deepcopy(self.message_history)

                if len(messages) >= 3:
                    if messages[-1]["role"] == "user" and isinstance(messages[-1]["content"], list) and messages[-1]["content"]:
                        messages[-1]["content"][-1]["cache_control"] = {"type": "ephemeral"}

                    if len(messages) >= 5 and messages[-3]["role"] == "user" and isinstance(messages[-3]["content"], list) and messages[-3]["content"]:
                        messages[-3]["content"][-1]["cache_control"] = {"type": "ephemeral"}

                response = self.client.messages.create(
                    model=MODEL_NAME,
                    max_tokens=MAX_TOKENS,
                    system=SYSTEM_PROMPT,
                    messages=messages,
                    tools=AVAILABLE_TOOLS,
                    temperature=TEMPERATURE,
                )

                self.logger.info(f"Response usage: {response.usage}")

                tool_calls = [
                    block for block in response.content if block.type == "tool_use"
                ]

                for block in response.content:
                    if block.type == "text":
                        self.logger.info(f"[Text] {block.text}")
                    elif block.type == "tool_use":
                        self.logger.info(f"[Tool] Using tool: {block.name}")

                if tool_calls:
                    assistant_content = []
                    for block in response.content:
                        if block.type == "text":
                            assistant_content.append({"type": "text", "text": block.text})
                        elif block.type == "tool_use":
                            assistant_content.append({"type": "tool_use", **dict(block)})

                    self.message_history.append(
                        {"role": "assistant", "content": assistant_content}
                    )

                    tool_results = []
                    for tool_call in tool_calls:
                        tool_result = self.process_tool_call(tool_call)
                        tool_results.append(tool_result)

                    self.message_history.append(
                        {"role": "user", "content": tool_results}
                    )

                    if len(self.message_history) >= self.max_history:
                        self.summarize_history()

                steps_completed += 1
                self.logger.info(f"Completed step {steps_completed}/{num_steps}")

            except KeyboardInterrupt:
                self.logger.info("Received keyboard interrupt, stopping")
                self.running = False
            except Exception as e:
                self.logger.error(f"Error in agent loop: {e}")
                raise e

        return steps_completed

    def summarize_history(self):
        self.logger.info("Generating conversation summary...")

        state_response = self._request_state()

        messages = copy.deepcopy(self.message_history)

        if len(messages) >= 3:
            if messages[-1]["role"] == "user" and isinstance(messages[-1]["content"], list) and messages[-1]["content"]:
                messages[-1]["content"][-1]["cache_control"] = {"type": "ephemeral"}

            if len(messages) >= 5 and messages[-3]["role"] == "user" and isinstance(messages[-3]["content"], list) and messages[-3]["content"]:
                messages[-3]["content"][-1]["cache_control"] = {"type": "ephemeral"}

        messages += [
            {
                "role": "user",
                "content": [
                    {
                        "type": "text",
                        "text": SUMMARY_PROMPT,
                    }
                ],
            }
        ]

        response = self.client.messages.create(
            model=MODEL_NAME,
            max_tokens=MAX_TOKENS,
            system=SYSTEM_PROMPT,
            messages=messages,
            temperature=TEMPERATURE
        )

        summary_text = " ".join([block.text for block in response.content if block.type == "text"])

        self.logger.info(f"Game Progress Summary:")
        self.logger.info(f"{summary_text}")

        summary_content = [
            {
                "type": "text",
                "text": f"CONVERSATION HISTORY SUMMARY (representing {self.max_history} previous messages): {summary_text}"
            },
            {
                "type": "text",
                "text": "\n\nCurrent game screenshot for reference:"
            },
        ]

        if state_response.screenshot:
            screenshot_b64 = get_screenshot_base64(state_response.screenshot, upscale=1)
            summary_content.append({
                "type": "image",
                "source": {
                    "type": "base64",
                    "media_type": "image/png",
                    "data": screenshot_b64,
                },
            })

        summary_content.append({
            "type": "text",
            "text": "You were just asked to summarize your playthrough so far, which is the summary you see above. You may now continue playing by selecting your next action."
        })

        self.message_history = [
            {"role": "user", "content": summary_content}
        ]

        self.logger.info("Message history condensed into summary.")

    def stop(self):
        self.running = False
