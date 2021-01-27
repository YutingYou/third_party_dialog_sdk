from .user_interface import UserInterface
from .cli import Cli

cli = Cli()

try:
    from .gui import Gui
    gui = Gui()
    active_interface = gui
except ImportError:
    gui = None
    active_interface = cli

suppress_errors = False

def error_decorator(func):
    def print_error(error_msg, details=''):
        global suppress_errors
        if active_interface is gui:
            cli.error(error_msg, details)
            if suppress_errors:
                return
        func(error_msg, details)
    return print_error

def set_suppress_errors(state):
    global suppress_errors
    suppress_errors = state

set_verbose = cli.set_verbose
print_title = cli.print_title
print_message = cli.print_message
print_header = cli.print_header
print_footer = cli.print_footer

info = active_interface.info
error = error_decorator(active_interface.error)
ask = active_interface.ask
ask_value = active_interface.ask_value
select_item = active_interface.select_item
fill_checkbox_list = active_interface.fill_checkbox_list

__all__ = ['print_title', 'print_header', 'print_footer', 'print_message', 'set_verbose',
           'select_item', 'ask', 'ask_value', 'info', 'fill_checkbox_list', 'set_suppress_errors']
