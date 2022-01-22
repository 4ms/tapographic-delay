#!/usr/bin/python2.5
#
# Copyright 2016 Matthias Puech.
#
# Author: Matthias Puech <matthias.puech@gmail.com>
# Based on code by: Olivier Gillet <ol.gillet@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# See http://creativecommons.org/licenses/MIT/ for more information.
#
# -----------------------------------------------------------------------------
#
# Lookup table definitions.

import numpy as np

import os
sample_rate = int(os.environ["SAMPLE_RATE"])

lookup_tables = []
int16_lookup_tables = []

"""----------------------------------------------------------------------------
XFade table
----------------------------------------------------------------------------"""

size = 17
t = np.arange(0, size) / float(size-1)
t = 1.04 * t - 0.02
t[t < 0] = 0
t[t >= 1] = 1
t *= np.pi / 2
lookup_tables.append(('xfade_in', np.sin(t)))
lookup_tables.append(('xfade_out', np.cos(t)))

"""----------------------------------------------------------------------------
Factory presets
----------------------------------------------------------------------------"""

def normalize(l):
    return l/max(l)

VEL_AMP=0
VEL_LP=1
VEL_BP=2

maxtaps = 32

slots = []

# Bank A: long bouncings

# 1
length = 5 # seconds
size = 16 # taps

t = normalize(np.add.accumulate(np.power(1.2, np.arange(1.0, size+1))))

slots.append({
    'size': size,
    'times': t*sample_rate*length,
    'velos': normalize(np.arange(1.0, size+1)),
    'types': np.repeat([VEL_LP], size),
    'pans': np.resize([0,1], size),
})

# 2
length = 6
t = normalize(np.add.accumulate(np.power(0.9, np.arange(1.0, size+1))))

slots.append({
    'size': size,
    'times': t*sample_rate*length,
    'velos': 1-normalize(np.arange(0.0, size)),
    'types': np.repeat([VEL_BP], size),
    'pans': np.resize([0,1], size),
})

# 3
length = 5
size = 30

t1 = normalize(np.power(1.3, np.arange(1.0, 16)))
t2 = t1[::-1]
t = normalize(np.add.accumulate(np.append(t1, t2)))

v1 = np.arange(1.0, 16)
v2 = v1[::-1]
v = normalize(np.append(v1, v2))

slots.append({
    'size': size,
    'times': t * sample_rate * length,
    'velos': v * v,
    'types': np.repeat([VEL_AMP], size),
    'pans': normalize(np.arange(0.0, size)),
})

# 4
length = 5
size = 30

t1 = normalize(np.power(0.8, np.arange(1.0, size/2+1)))
t2 = t1[::-1]
t = normalize(np.add.accumulate(np.append(t1, t2)))

v1 = np.arange(1.0, size/2+1)
v2 = v1[::-1]
v = 1 - normalize(np.append(v1, v2))

slots.append({
    'size': size,
    'times': t * sample_rate * length,
    'velos': v,
    'types': np.repeat([VEL_BP], size),
    'pans': 1-normalize(np.arange(0.0, size)),
})

# 5
length = 3
size = 16

t = normalize(np.add.accumulate(np.power(0.99, np.arange(1.0, size+1))))
v = normalize(np.power(1.2, np.arange(1.0, size+1)))

slots.append({
    'size': size,
    'times': t * sample_rate * length,
    'velos': v,
    'types': np.repeat([VEL_AMP], size),
    'pans': np.random.uniform(size=size),
})

# 6
length = 3
size = 16

t = normalize(np.add.accumulate(np.power(1.005, np.arange(1.0, size+1))))
v1 = normalize(np.power(0.6, np.arange(1.0, size/2)))
v2 = v1[::-1]
v = normalize(np.append(v1, v2))

slots.append({
    'size': size,
    'times': t * sample_rate * length,
    'velos': v,
    'types': np.repeat([VEL_LP], size),
    'pans': normalize(np.arange(0.0, size)),
})

# Bank B:

# 1
t = [0,1,0,2,1,0,0,3,0]
t = np.array(map(float, t))
t = normalize(np.add.accumulate(t+1))
v = [0.1,1,0.3,0.1,1,0.3,1,0.3,0.1]
size = len(t)

slots.append({
    'size': size,
    'times': t * sample_rate * size / 4,
    'velos': v,
    'types': np.repeat([VEL_LP], size),
    'pans': np.resize([0,1], size),
})

# 2
t = [1,0,0,0,3,2,0,1,1,0,1,0]
t = np.array(map(float, t))
t = normalize(np.add.accumulate(t+1))
v = [0.1,0.3,0.7,0.55,0.2,0.5,0.9,0.45,0.6,0.1,0.9,0.2]
size = len(t)

slots.append({
    'size': size,
    'times': t * sample_rate * size / 4,
    'velos': v,
    'types': np.repeat([VEL_BP], size),
    'pans': np.resize([0,1], size),
})

# 3
t = [1,1,1,1,0,0,0,0,4,4,4,4,2,2,2,2]
t = np.array(map(float, t))
t = normalize(np.add.accumulate(t+1))
v = [0.5,0.5,1,1,1,1,0.5,0.5,0.5,0.5,1,1,1,1,1,1]
size = len(t)

slots.append({
    'size': size,
    'times': t * sample_rate * size / 4,
    'velos': v,
    'types': np.repeat([VEL_AMP], size),
    'pans': np.resize([0,1], size),
})

# 4
t = [0,0,0,0,1,1,1,1,2,2,2,2,4,4,4,4]
t = np.array(map(float, t))
t = normalize(np.add.accumulate(t+1))
v = np.resize([0.35,0.5,0.65,0.8], size)
size = len(t)

slots.append({
    'size': size,
    'times': t * sample_rate * size / 2,
    'velos': v,
    'types': np.repeat([VEL_BP], size),
    'pans': np.resize([0,1], size),
})

# 5
t = [2,2,2,0,0,0,1,1,1,0,0,0,3,3,3]
t = np.array(map(float, t))
t = normalize(np.add.accumulate(t+1))
v = np.resize([0,0.5,1], size)
size = len(t)

slots.append({
    'size': size,
    'times': t * sample_rate * size / 4,
    'velos': v,
    'types': np.repeat([VEL_LP], size),
    'pans': np.resize([0,1], size),
})

# 6
t = [0,0,0,2,0,0,0,4,0,0,0,8,0,0,0]
t = np.array(map(float, t))
t = normalize(np.add.accumulate(t+1))
v = np.resize([0,0.33, 0.66,1], size)
size = len(t)

slots.append({
    'size': size,
    'times': t * sample_rate * size / 4,
    'velos': v,
    'types': np.repeat([VEL_AMP], size),
    'pans': np.resize([0,1], size),
})

# Bank C: Random / diffusion IRs

phi = 1.61803398875

# 1: golden ratio
slots.append({
    'size': 3,
    'times': np.array([1, phi, phi * phi]) * sample_rate * 1,
    'velos': [1, 1, 1],
    'types': np.repeat([VEL_AMP], 3),
    'pans': [0, 1, 0.5],
})

# 2: optimally diffusing 6-taps
slots.append({
    'size': 6,
    'times': np.array([0.11772496, 0.30086703, 0.398813414, 0.545568,
                       0.750073, 1.0]) * sample_rate * 4,
    'velos': [1, 1./2, 1./3, 1./4, 1./5, 1./6],
    'types': np.repeat([VEL_LP], 6),
    'pans': [0, 1, 0, 1, 0, 1],
})

# 3: crescendo with random times
size = 16

ts = np.random.uniform(size=size)
ts = normalize(np.add.accumulate(ts))
vs = ts

slots.append({
    'size': size,
    'times': ts * sample_rate * 3,
    'velos': vs,
    'types': np.repeat([VEL_BP], size),
    'pans': np.random.uniform(size=size),
})

# 4: decrescendo with random times
ts = np.random.uniform(size=size)
ts = normalize(np.add.accumulate(ts))
vs = ts

slots.append({
    'size': size,
    'times': ts * sample_rate * 3,
    'velos': 1-vs,
    'types': np.repeat([VEL_AMP], size),
    'pans': np.random.uniform(size=size),
})

# 5:
ts = np.random.uniform(size=size)
ts = normalize(np.sort(ts))
vs = np.random.uniform(size=size)

slots.append({
    'size': size,
    'times': ts * sample_rate * 5,
    'velos': vs,
    'types': np.repeat([VEL_BP], size),
    'pans': normalize(np.arange(0.0, size)),
})

# 6: all random
slots.append({
    'size': size,
    'times': np.random.uniform(size=size) * sample_rate * 3,
    'velos': np.random.uniform(size=size),
    'types': np.random.randint(0, high=3, size=size),
    'pans': np.random.uniform(size=size),
})

# Bank D:
# (left intentionally empty)
slots.append({ 'size': 0, 'times': [], 'velos': [], 'types': [], 'pans': []})
slots.append({ 'size': 0, 'times': [], 'velos': [], 'types': [], 'pans': []})
slots.append({ 'size': 0, 'times': [], 'velos': [], 'types': [], 'pans': []})
slots.append({ 'size': 0, 'times': [], 'velos': [], 'types': [], 'pans': []})
slots.append({ 'size': 0, 'times': [], 'velos': [], 'types': [], 'pans': []})
slots.append({ 'size': 0, 'times': [], 'velos': [], 'types': [], 'pans': []})

# # Graph all IRs:
# import matplotlib.pyplot as plt

# for i in range(len(slots)):
#     plt.figure(figsize=(10,7))
#     slot = slots[i]
#     s = zip(slot['times'], slot['velos'])
#     for (x,y) in s:
#         plt.axvline(x, ymax=y)
#     plt.savefig("slot"+str(i)+".pdf")

# Formatting and writing to table

def pad(a):
    return np.append(a, np.repeat([0], maxtaps - len(a)))

times, velos, pans, types, sizes = [], [], [], [], []

for slot in slots:
    sizes.append(slot['size'])
    times.extend(pad(slot['times']))
    velos.extend(pad(slot['velos']))
    types.extend(pad(slot['types']))
    pans.extend(pad(slot['pans']))

lookup_tables.append(('preset_times', times))
lookup_tables.append(('preset_velos', velos))
lookup_tables.append(('preset_pans', pans))
int16_lookup_tables.append(('preset_types', types))
int16_lookup_tables.append(('preset_sizes', sizes))
