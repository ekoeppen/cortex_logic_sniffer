#!/usr/bin/python
# coding=iso8859-1

from Tkinter import *

frequency = 72000000.0
timer_interval = 0x01000000
pixel_per_microsecond = 1

class Channel:
    def __init__(self):
        self.samples = []

    def add_sample(self, value, timestamp):
        self.samples.append((value, timestamp))

    def last_state(self):
        if len(self.samples) > 0:
            return self.samples[-1][0]
        else:
            return None

    def print_samples(self):
        for s in self.samples:
            print("%1d %10d" % (s[0], s[1]))

class LogicAnalyzer:
    max_channels = 8
    timer_interval = 0x01000000
    frequency = 72000000
    timebase = 1000000000

    def __init__(self):
        self.channels = []
        for i in range(self.max_channels):
            self.channels.append(Channel())

    def normalize(self, timestamp):
        return timestamp * (self.timebase / self.frequency)

    def load(self, filename):
        f = open(filename)
        t = 0
        for line in f:
            what = (int(line, 16) >> 24) & 0xff
            timestamp = self.timer_interval - (int(line, 16) & 0x00ffffff)
            if what & 0x80:
                t += self.timer_interval
            else:
                m = 1
                for i in range(len(self.channels)):
                    state = ((what & m) != 0)
                    if state != self.channels[i].last_state():
                        self.channels[i].add_sample(state, self.normalize(t + timestamp))
                    m <<= 1
        f.close()

    def print_channels(self):
        for c in range(len(self.channels)):
            print("Channel %d -----------------------------------------------------------" % c)
            self.channels[c].print_samples()

class ChannelView(Canvas):
    bg_color = "#003e4d"
    width = 800
    height = 480
    channel_colors = ["#e00000", "#a0a0a0", "white", "white", "white", "white", "white", "white"]

    def __init__(self, parent, delegate, analyzer):
        Canvas.__init__(self, parent, bg = self.bg_color, width = self.width, height = self.height)
        self.delegate = delegate
        self.analyzer = analyzer
        self.last_x = 0
        self.divider = 100000
        self.cursor_line = self.create_line(10, 0, 10, 10000, fill = "green", dash = (4, 4))
        self.bind("<Motion>", self.track_mouse)

    def plot_channel(self, channel, x_offset, y_offset, color):
        global scale_factor
        x = x_offset
        y = y_offset
        for sample in channel.samples:
            x0 = x
            y0 = y
            if sample[0]:
                y = y_offset
            else:
                y = y_offset + 50
            x = (sample[1]) / self.divider + x_offset
            self.create_line(x0, y0, x, y0, fill = color, width = 2, tags = "sample")
            self.create_line(x, y0, x, y, fill = color, width = 2, tags = ("c", "sample"))
        self.last_x = max(x, self.last_x)

    def plot_channels(self):
        self.delete("sample")
        self.last_x = 0
        for i in range(len(self.analyzer.channels)):
            self.plot_channel(self.analyzer.channels[i], 0, i * 50 + 10, self.channel_colors[i])
        self.config(width = self.last_x)
        self.config(scrollregion = (0, 0, self.last_x, 480))

    def set_cursor_line(self, pos):
        self.coords(self.cursor_line, self.canvasx(pos), 0, self.canvasx(pos), 10000)

    def track_mouse(self, event):
        self.set_cursor_line(event.x)
        self.delegate.cursor_moved(self.canvasx(event.x) * float(self.divider))

    def rescale(self, value):
        old_scrollpos = self.canvasx(0) / int(self['width'])
        self.divider = 100000 / int(value)
        self.plot_channels()
        self.xview_moveto(old_scrollpos)

class Viewer:

    def __init__(self, parent, analyzer):
        self.cursor_pos = StringVar()
        self.analyzer = analyzer
        topframe = Frame(parent)
        topframe.pack(fill = X)

        scale = Scale(topframe, orient = "horizontal", length = 400, from_ = 1, to = 1000)
        scale.pack(side = LEFT, fill = X, padx = 10)
        Label(topframe, textvariable = self.cursor_pos).pack(side = LEFT, padx = 10)
        Label(topframe).pack(side = LEFT, expand = 1)
        Button(topframe, text = "Quit", command = quit).pack(side = LEFT)
        Button(topframe, text = "Reload", command = self.reload_samples).pack(side = LEFT, padx = 5, pady = 5)

        self.channel_view = ChannelView(parent, self, self.analyzer)
        self.channel_view.pack(fill = BOTH, expand = 1)
        hbar = Scrollbar(parent, orient = HORIZONTAL)
        hbar.pack(fill = X)
        hbar.config(command = self.channel_view.xview)
        self.channel_view.config(xscrollcommand = hbar.set)
        scale.bind("<ButtonRelease-1>", lambda e: self.channel_view.rescale(e.widget.get()))
        self.cursor_pos.set("Pos: %.0fµs" % 0.0)
        scale.set(pixel_per_microsecond)

        parent.bind("n", self.next_level_change)
        parent.bind("q", lambda e: exit())
        parent.bind("r", lambda e: self.reload_samples())

    def cursor_moved(self, pos):
        self.cursor_pos.set("Pos: %.0fµs" % (pos / 1000, ))

    def next_level_change(self, event):
        smallest_x = int(self.canvas['width'])
        n = -1
        edge = self.canvas.canvasx(0) + root.winfo_width()
        for c in self.canvas.find_withtag(ALL):
            x = self.canvas.coords(c)[0]
            if x > edge and x < smallest_x:
                smallest_x = x
                n = c
        if n != -1:
            self.canvas.xview_moveto(canvas.coords(n)[0] / int(self.canvas['width']))

    def reload_samples(self):
        None

root = Tk()

analyzer = LogicAnalyzer()
analyzer.load(sys.argv[1])
viewer = Viewer(root, analyzer)
viewer.channel_view.plot_channels()

root.mainloop()
