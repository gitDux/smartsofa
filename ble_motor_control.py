#!/usr/bin/env python3
"""CH571R BLE dual-motor controller.

Install dependency:
    pip install bleak

Examples:
    python ble_motor_control.py 1 forward 200
    python ble_motor_control.py 2 reverse 160
    python ble_motor_control.py 1 stop
    python ble_motor_control.py --interactive
"""

import argparse
import asyncio

from bleak import BleakClient, BleakScanner


DEVICE_NAME = "BLE Motor"
MOTOR_CHAR_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"
DIRECTIONS = {"stop": 0, "forward": 1, "reverse": 2}


async def find_controller(address: str | None):
    if address:
        return address

    print(f"Scanning for {DEVICE_NAME}...")
    devices = await BleakScanner.discover(timeout=8.0)
    for device in devices:
        if device.name == DEVICE_NAME:
            return device
    raise RuntimeError(f"BLE device '{DEVICE_NAME}' was not found")


async def write_motor(client: BleakClient, motor: int, direction: str, speed: int):
    if motor not in (1, 2):
        raise ValueError("motor must be 1 or 2")
    if direction not in DIRECTIONS:
        raise ValueError("direction must be stop, forward, or reverse")
    if not 0 <= speed <= 255:
        raise ValueError("speed must be in the range 0..255")

    if direction == "stop":
        speed = 0
    elif speed == 0:
        raise ValueError("speed must be greater than 0 while the motor is running")

    payload = bytes((motor, DIRECTIONS[direction], speed))
    await client.write_gatt_char(MOTOR_CHAR_UUID, payload, response=True)
    print(f"Sent: motor={motor}, direction={direction}, speed={speed}")


async def interactive(client: BleakClient):
    print("Commands: <motor 1|2> <forward|reverse|stop> [speed 0..255], or quit")
    while True:
        command = await asyncio.to_thread(input, "motor> ")
        parts = command.strip().lower().split()
        if not parts:
            continue
        if parts[0] in ("quit", "exit", "q"):
            return
        try:
            if len(parts) not in (2, 3):
                raise ValueError("example: 1 forward 200")
            motor = int(parts[0])
            direction = parts[1]
            speed = int(parts[2]) if len(parts) == 3 else (0 if direction == "stop" else 255)
            await write_motor(client, motor, direction, speed)
        except (ValueError, OSError) as exc:
            print(f"Invalid command: {exc}")


async def main(args):
    controller = await find_controller(args.address)
    async with BleakClient(controller) as client:
        print("Connected")
        if args.interactive:
            await interactive(client)
        else:
            speed = args.speed if args.speed is not None else (0 if args.direction == "stop" else 255)
            await write_motor(client, args.motor, args.direction, speed)


def parse_args():
    parser = argparse.ArgumentParser(description="Control the CH571R BLE dual-motor board")
    parser.add_argument("motor", type=int, nargs="?", choices=(1, 2))
    parser.add_argument("direction", nargs="?", choices=tuple(DIRECTIONS))
    parser.add_argument("speed", type=int, nargs="?")
    parser.add_argument("--address", help="BLE MAC address/UUID; skips name scanning")
    parser.add_argument("-i", "--interactive", action="store_true", help="keep the BLE connection open")
    args = parser.parse_args()
    if not args.interactive and (args.motor is None or args.direction is None):
        parser.error("motor and direction are required unless --interactive is used")
    return args


if __name__ == "__main__":
    asyncio.run(main(parse_args()))
