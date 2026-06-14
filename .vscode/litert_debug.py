#!/usr/bin/env python3
"""
LiteRT debugging helpers for LLDB
This script provides custom LLDB commands for debugging LiteRT applications
"""

import lldb

def __lldb_init_module(debugger, internal_dict):
    """Initialize the debugging module"""
    debugger.HandleCommand('command script add -f litert_debug.print_tensor_info tensor_info')
    debugger.HandleCommand('command script add -f litert_debug.print_model_info model_info')
    debugger.HandleCommand('command script add -f litert_debug.print_buffer_info buffer_info')
    print('LiteRT debugging helpers loaded successfully!')
    print('Available commands:')
    print('  tensor_info <variable>  - Print tensor information')
    print('  model_info <variable>   - Print model information')
    print('  buffer_info <variable>  - Print buffer information')

def print_tensor_info(debugger, command, result, internal_dict):
    """Print information about a LiteRT tensor"""
    target = debugger.GetSelectedTarget()
    process = target.GetProcess()
    thread = process.GetSelectedThread()
    frame = thread.GetSelectedFrame()

    if not command:
        result.AppendMessage("Usage: tensor_info <variable_name>")
        return

    var = frame.FindVariable(command.strip())
    if not var.IsValid():
        result.AppendMessage(f"Variable '{command.strip()}' not found")
        return

    result.AppendMessage(f"=== Tensor Info: {command.strip()} ===")
    result.AppendMessage(f"Address: {var.GetLocation()}")
    result.AppendMessage(f"Type: {var.GetType()}")
    result.AppendMessage(f"Value: {var.GetValue()}")

    # Try to get tensor dimensions if it's a LiteRT tensor
    try:
        # This would need to be adapted based on actual LiteRT tensor structure
        result.AppendMessage("LiteRT tensor details would be shown here")
    except:
        result.AppendMessage("Could not parse tensor details")

def print_model_info(debugger, command, result, internal_dict):
    """Print information about a LiteRT model"""
    target = debugger.GetSelectedTarget()
    process = target.GetProcess()
    thread = process.GetSelectedThread()
    frame = thread.GetSelectedFrame()

    if not command:
        result.AppendMessage("Usage: model_info <variable_name>")
        return

    var = frame.FindVariable(command.strip())
    if not var.IsValid():
        result.AppendMessage(f"Variable '{command.strip()}' not found")
        return

    result.AppendMessage(f"=== Model Info: {command.strip()} ===")
    result.AppendMessage(f"Address: {var.GetLocation()}")
    result.AppendMessage(f"Type: {var.GetType()}")

    # Try to extract model information
    try:
        result.AppendMessage("Model structure details would be shown here")
    except:
        result.AppendMessage("Could not parse model details")

def print_buffer_info(debugger, command, result, internal_dict):
    """Print information about a tensor buffer"""
    target = debugger.GetSelectedTarget()
    process = target.GetProcess()
    thread = process.GetSelectedThread()
    frame = thread.GetSelectedFrame()

    if not command:
        result.AppendMessage("Usage: buffer_info <variable_name>")
        return

    var = frame.FindVariable(command.strip())
    if not var.IsValid():
        result.AppendMessage(f"Variable '{command.strip()}' not found")
        return

    result.AppendMessage(f"=== Buffer Info: {command.strip()} ===")
    result.AppendMessage(f"Address: {var.GetLocation()}")
    result.AppendMessage(f"Type: {var.GetType()}")

    # Try to show buffer contents
    try:
        result.AppendMessage("Buffer contents would be shown here")
    except:
        result.AppendMessage("Could not parse buffer contents")