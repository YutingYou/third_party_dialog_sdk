#########################################################################################
# Copyright (C) 2015-2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import tkinter as tk
import re

from .user_interface import UserInterface


class Gui(UserInterface):

    MIN_WIDTH = 200
    MIN_HEIGHT = 0

    def info(self, text):
        root = tk.Tk()
        root.title('INFO')
        root.minsize(width=Gui.MIN_WIDTH, height=Gui.MIN_HEIGHT)
        root.resizable(False, False)
        root.geometry('+{}+{}'.format(0, 0))
        tk.Label(root, text=text, justify=tk.LEFT).pack(padx=10, pady=10)
        tk.Button(text="OK", command=lambda: root.destroy()).pack(side=tk.BOTTOM, padx=10, pady=10)
        tk.mainloop()

    def error(self, text='', details=''):
        text = text if text else 'Unknown error'
        root = tk.Tk()
        root.title('ERROR')
        root.minsize(width=Gui.MIN_WIDTH, height=Gui.MIN_HEIGHT)
        root.resizable(False, False)
        root.geometry('+{}+{}'.format(0, 0))
        tk.Label(root, text=text, justify=tk.LEFT, anchor=tk.W).pack(padx=10, pady=10, fill=tk.BOTH)

        if details:
            f = tk.Frame(root)
            f.pack(padx=5, pady=5, fill=tk.BOTH)

            width = max(map(len, details.splitlines()))
            if width > 70:
                width = 70

            height = details.count('\n') + 1
            if height > 20:
                height = 20

            y_scroll_bar = tk.Scrollbar(f)
            x_scroll_bar = tk.Scrollbar(f, orient=tk.HORIZONTAL)
            tbox = tk.Text(f, height=height, width=width, wrap=tk.NONE,
                           yscrollcommand=y_scroll_bar.set, xscrollcommand=x_scroll_bar.set)

            y_scroll_bar.pack(side=tk.RIGHT, fill=tk.Y)
            x_scroll_bar.pack(side=tk.BOTTOM, fill=tk.X)
            tbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

            tbox.insert(tk.INSERT, details)

            y_scroll_bar.config(command=tbox.yview)
            x_scroll_bar.config(command=tbox.xview)
            tbox.config(state=tk.DISABLED)

        tk.Button(text="OK", command=lambda: root.destroy()).pack(side=tk.BOTTOM, padx=10, pady=10)
        tk.mainloop()

    def ask(self, text, confirmation='Yes', denial='No'):
        root = tk.Tk()
        root.title('Answer question')
        root.minsize(width=Gui.MIN_WIDTH, height=Gui.MIN_HEIGHT)
        root.resizable(False, False)
        root.geometry('+{}+{}'.format(0, 0))
        ret_val = tk.BooleanVar()
        tk.Label(root, text=text, justify=tk.LEFT).pack(padx=10, pady=10)

        def confirmation_cb():
            ret_val.set(True)
            root.destroy()

        def denial_cb():
            ret_val.set(False)
            root.destroy()

        tk.Button(text=confirmation, command=confirmation_cb).pack(side=tk.RIGHT, padx=5, pady=5)
        tk.Button(text=denial, command=denial_cb).pack(side=tk.RIGHT, padx=5, pady=5)

        tk.mainloop()
        return bool(ret_val.get())

    def ask_value(self, value_name, text='Insert value', default='', regex=None):
        root = tk.Tk()
        root.title('Insert value')
        root.minsize(width=Gui.MIN_WIDTH, height=Gui.MIN_HEIGHT)
        root.resizable(False, False)
        root.geometry('+{}+{}'.format(0, 0))

        ret_val = tk.StringVar()

        tk.Label(root, text=text, justify=tk.LEFT).pack(padx=10, pady=10)

        value_frame = tk.Frame()
        tk.Label(value_frame, text=value_name + ': ', justify=tk.CENTER).pack(side=tk.LEFT)
        text = tk.Entry(value_frame, justify=tk.LEFT)
        text.insert(tk.END, default)
        text.pack(side=tk.RIGHT)
        value_frame.pack(padx=10, pady=10)

        error_msg = tk.StringVar('')
        error_label = tk.Label(root, textvariable=error_msg, justify=tk.LEFT)
        error_label.pack(padx=10, pady=10)

        def ok():
            input_value = text.get().strip()

            if not input_value:
                error_msg.set('Input value can not be empty. Try Again')
                root.update_idletasks()
                return

            if regex and not re.fullmatch(regex, input_value):
                error_msg.set("Value does not match format: '{}'".format(regex))
                root.update_idletasks()
                return

            ret_val.set(input_value)
            root.destroy()

        def cancel():
            root.destroy()

        tk.Button(text='OK', command=ok).pack(side=tk.RIGHT, padx=5, pady=5)
        tk.Button(text='Cancel', command=cancel).pack(side=tk.RIGHT, padx=5, pady=5)

        tk.mainloop()
        return ret_val.get()

    def select_item(self, item_list, text='Select item', default=None):
        root = tk.Tk()
        root.title('Select item')
        root.minsize(width=Gui.MIN_WIDTH, height=Gui.MIN_HEIGHT)
        root.resizable(False, False)
        root.geometry('+{}+{}'.format(0, 0))
        ret_val = tk.StringVar()
        tk.Label(root, text=text, justify=tk.LEFT).pack(padx=10, pady=10)

        listbox = tk.Listbox(root)
        listbox.pack(padx=10, pady=10, expand=1, fill=tk.BOTH)
        listbox.config(width=max([len(str(e)) for e in item_list]))

        for item in item_list:
            listbox.insert(tk.END, item)

        def ok():
            ret_val.set(item_list[listbox.curselection()[0]])
            root.destroy()

        def cancel():
            root.destroy()

        ok_button = tk.Button(text='OK', command=ok)
        ok_button.pack(side=tk.RIGHT, padx=5, pady=5)
        tk.Button(text='Cancel', command=cancel).pack(side=tk.RIGHT, padx=5, pady=5)

        if default is not None:
            listbox.selection_set(default)
        else:
            listbox.selection_set(0)

        listbox.bind('<Double-1>', lambda x: ok_button.invoke())
        tk.mainloop()

        return ret_val.get()

    def fill_checkbox_list(self, values_dict, text='Fill checkbox list'):
        root = tk.Tk()
        root.title('Fill checkbox list')
        root.minsize(width=Gui.MIN_WIDTH, height=Gui.MIN_HEIGHT)
        root.resizable(False, False)
        root.geometry('+{}+{}'.format(0, 0))

        tk.Label(root, text=text, justify=tk.LEFT).pack(padx=5, pady=5)

        ret_val = dict()
        canceled = tk.BooleanVar(value=True)

        f = tk.Frame(root, borderwidth=1)
        for name in values_dict:
            ret_val[name] = tk.BooleanVar(value=values_dict[name])
            cb = tk.Checkbutton(f, text=name, justify=tk.LEFT, variable=ret_val[name])
            cb.pack(anchor=tk.W, padx=2, pady=2)
        f.pack(padx=5, pady=5, fill=tk.BOTH)

        def ok():
            canceled.set(False)
            root.destroy()

        def cancel():
            root.destroy()

        ok_button = tk.Button(text='OK', command=ok)
        ok_button.pack(side=tk.RIGHT, padx=5, pady=5)
        tk.Button(text='Cancel', command=cancel).pack(side=tk.RIGHT, padx=5, pady=5)

        tk.mainloop()
        return None if canceled.get() else {k: v.get() for k, v in ret_val.items()}
