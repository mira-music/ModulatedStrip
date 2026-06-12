---
title: "MODULATED STRIP VST"
subtitle: "Analog Channel Processor"
version: "v1.0.0"
date: "June 2026"
---

# MODULATED STRIP VST

**Analog Channel Processor for Ableton Live**

Version 1.0.0 — June 2026

---

## Overview

Modulated Strip is a professional analog channel strip VST plugin
that models the complete signal chain of a recording studio.

Pick your analog flavor at every stage independently:

- **7 saturation circuits** from legendary hardware
- **5 compressor models** with authentic behavior
- **5 equalizer models** with color EQ character
- **60+ factory presets** across 10 categories
- Built for **electronic music** and **live performance**

---

## Quick Start
Run ModulatedStrip_Setup_v1.0.0.exe (admin required)
Open Ableton Live
Options → Preferences → Plugins → Rescan
Find MODULATED STRIP in your browser
Drop on any track
Click the preset name → pick a preset
Press play
text


---

## System Requirements

| Requirement | Minimum | Recommended |
|-------------|---------|-------------|
| **OS** | Windows 10 (64-bit) | Windows 11 (64-bit) |
| **CPU** | Intel i5 / AMD Ryzen 5 | Intel i7 / AMD Ryzen 7 |
| **RAM** | 4 GB | 8 GB |
| **DAW** | Ableton Live 11 | Ableton Live 12 |
| **Format** | VST3 | VST3 |

---

## Installation

### Installer (Recommended)
Double-click ModulatedStrip_Setup_v1.0.0.exe
Follow the wizard
Plugin installs to: C:\Program Files\Common Files\VST3\
Rescan plugins in your DAW
text


### Manual Install
Copy the "Modulated Strip.vst3" folder to:
C:\Program Files\Common Files\VST3\

text


### Uninstall
Windows Settings → Apps → Modulated Strip → Uninstall
User presets are preserved in AppData

text


---

## Signal Chain
INPUT GAIN → SATURATION → COMPRESSOR → EQ → OUTPUT GAIN
↕
(EQ PRE switch reverses
compressor and EQ order)

text


---

## Hardware Models

### Saturation (7 Models)

| Model | Based On | Sound |
|-------|----------|-------|
| NEVE | 1073 transformer + Class A | Thick, warm, musical |
| SSL | Console VCA chip | Clean, precise, subtle |
| API | 2520 discrete op-amp | Forward, exciting, punchy |
| TUBE | 12AX7 vacuum tube | Warm, vintage, asymmetric |
| TAPE | Ampex/Studer oxide | Smooth, cohesive, natural |
| FET | JFET transistor | Aggressive, fast, gritty |
| IRON | Output transformer | Subtle, transparent, 3D |

### Compressors (5 Models)

| Model | Based On | Sound |
|-------|----------|-------|
| SSL Bus | SSL G-Series VCA | Punchy glue, modern records |
| Fairchild 670 | Variable-mu tube | Smoothest ever made |
| LA-2A | T4B electro-optical | Transparent, musical |
| 1176 | UREI FET | Fastest, aggressive, punchy |
| API 2500 | VCA + Thrust HPF | Dense, bass-preserving |

### Equalizers (5 Models)

| Model | Based On | Sound |
|-------|----------|-------|
| Neve 1073 | Inductor + Class A | Musical warmth, bloom |
| Neve 1084 | Extended 1073 | More flexible, surgical |
| SSL 4000E | State Variable Filter | Clean, precise |
| Pultec EQP-1A | Passive LC + tube | Phase bloom, vintage |
| API 550A | Discrete Class A | Aggressive, focused |

---

## Features

### Core Processing

- **Oversampling:** 1x / 2x / 4x / 8x on saturation stage
- **Delta Mode:** Hear only what the plugin changes
- **Analog Bypass:** Transformer color only, all processing bypassed
- **EQ Pre/Post:** Switch EQ before or after compressor
- **Per-Section Bypass:** Toggle each stage independently

### Stereo Processing

- **Stereo Linked:** Both channels processed identically
- **Mid/Side:** Process center and width independently
- **Dual Mono:** Each channel fully independent

### Analog Authenticity

- **Inter-channel Crosstalk:** 0.2% bleed like real hardware
- **Model Noise Floor:** Correct noise level per model type
- **Output Soft Clipper:** Transparent tanh at -0.5 dBFS

### Metering

- **LED Ladder Meters:** Input and output with peak hold
- **Clip Latch LEDs:** Latching red indicator, click to clear
- **VU Needle Meter:** Physics-based GR needle with model-specific face
- **GR History Trace:** 2-second compression visualization
- **EQ Curve Display:** Real-time frequency response
- **LUFS Meter:** Short-term loudness display

### Preset System

- **60+ Factory Presets** across 10 categories
- **User Preset Save/Load** with categories and descriptions
- **Preset Browser** with filtering and search
- **A/B Comparison** — store and switch between two states

---

## Factory Preset Categories

| Category | Count | Description |
|----------|-------|-------------|
| Deep House | 6 | Bus, kick, bass, pads, modular, groove |
| Progressive House | 4 | Bus, lead, pluck, atmosphere |
| Melodic House | 4 | Bus, chords, arp, emotion |
| Techno | 3 | Dark, kick, bus |
| Ambient | 2 | Pad, drone |
| Synths | 3 | Bass punch, analog color, 808 |
| Vocals | 4 | Smooth, aggressive, radio, warm |
| Drums | 2 | Punch bus, vintage bus |
| Live Performance | 6 | Opener, warmup, build, peak, transition, after hours |
| Mastering | 4 | Analog, modern, vintage, streaming |
| Mixing | 3 | Reference, parallel drums, synth bus |
| Creative | 3 | Heavy saturation, tape machine, lo-fi |

---

## Quick Recipes

### Deep House Mix Bus
SAT: NEVE Drive 15% Mix 80%
COMP: SSL Bus 2:1 -20dB 3ms 400ms Mix 65%
EQ: Neve 1073 Low +2 at 60Hz Mid -1 at 350Hz High +2.5 at 12kHz

text


### The Pultec Bass Trick
SAT: TUBE Drive 15% Mix 75%
COMP: LA-2A COMPRESS 35%
EQ: Pultec LF boost +6dB at 60Hz (auto-cut creates magic)

text


### Parallel Drum Bus
SAT: FET Drive 35% Mix 60%
COMP: 1176 8:1 fast attack 80ms release Mix 40%
EQ: API 550A Low +3 Mid +2 at 3kHz

text


### Live Peak Time
SAT: FET Drive 45% Mix 85%
COMP: API 2500 THRUST ON 4:1 -16dB Mix 75%
EQ: API 550A Low +4 at 80Hz Mid -2 at 500Hz High +3 at 10kHz

text


---

## Technical Specifications

| Specification | Value |
|---------------|-------|
| Format | VST3 |
| Channels | Stereo (Linked / M/S / Dual Mono) |
| Sample Rates | 44.1k – 192k Hz |
| Bit Depth | 32-bit float internal |
| Oversampling | 1x / 2x / 4x / 8x |
| Latency | 0 samples (1x) to ~256 (8x) |
| Window Size | 1280 × 660 pixels |
| Presets | 60+ factory, unlimited user |
| DSP Accuracy | 94/100 (independent review) |

### CPU Usage (per instance, 44.1kHz, modern i7)

| Settings | CPU |
|----------|-----|
| Light | 1.5 – 2.5% |
| Medium | 3 – 4% |
| Heavy | 5 – 7% |
| Heavy + 4x OS | 8 – 12% |

---

## File Locations

| File | Path |
|------|------|
| Plugin | `C:\Program Files\Common Files\VST3\Modulated Strip.vst3` |
| User Presets | `C:\Users\[Name]\AppData\Roaming\ModulatedStrip\` |

---

## Known Limitations (v1.0.0)

| Limitation | Status |
|-----------|--------|
| Window not resizable | Planned v1.1 |
| No MIDI learn | Planned v1.1 |
| No macro knobs | Planned v1.1 |
| No spectrum analyzer | Planned v1.1 |
| No skin system | Planned v1.2 |
| Oversampling 4x/8x is CPU heavy | Use for renders only |

---

## Support
Email: [modulated.music.ofc@gmail.com]
Discord: [miraofc2380_08940]

text


---

## License

See LICENSE.txt for full terms.

Copyright 2026. All rights reserved.

---

## Documentation

See **TESTER_GUIDE.md** for complete documentation including:

- Detailed model descriptions with harmonic profiles
- Complete compressor physics explanations
- Famous signal chain combinations
- Live performance setup guide
- Full testing checklist
- Tips, tricks, and common mistakes

---

*Built with JUCE Framework*
*Designed for electronic music production and live performance*
*Modulated Strip v1.0.0 Beta*
