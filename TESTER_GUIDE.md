---
title: "MODULATED STRIP VST — Beta Tester Guide"
subtitle: "Analog Channel Processor for Ableton Live"
version: "v1.0.0"
date: "June 2026"
author: "Modulated Strip Development Team"
geometry: margin=2.5cm
fontsize: 11pt
linestretch: 1.15
toc: true
toc-depth: 3
numbersections: true
header-includes:
  - \usepackage{fancyhdr}
  - \pagestyle{fancy}
  - \fancyhead[L]{Modulated Strip VST}
  - \fancyhead[R]{v1.0.0 Beta}
  - \fancyfoot[C]{\thepage}
  - \fancyfoot[R]{Confidential — Do Not Distribute}
---

\newpage

# Welcome

Thank you for testing **Modulated Strip**.

This plugin is an analog channel strip modeled on the most legendary studio hardware ever built. It is designed specifically for **electronic music live performance** and **studio production**.

Your feedback shapes every future version. The more specific your feedback, the better the plugin becomes.

> **Important:** This is a beta release for testing purposes only. Do not redistribute this software.

\newpage

# What This Plugin Does

Modulated Strip puts the **complete signal chain of a professional analog recording studio** in one VST plugin.

The concept is unique: **pick your analog flavor at every stage independently**. No other plugin lets you combine these specific hardware models freely.

## The Full Model Library

### Saturation (7 Models)

| Model | Based On | Character |
|-------|----------|-----------|
| **NEVE** | 1073 transformer + Class A | Thick, warm, musical |
| **SSL** | Console VCA chip | Clean, precise, subtle |
| **API** | 2520 discrete op-amp | Forward, exciting, punchy |
| **TUBE** | 12AX7 vacuum tube | Warm, vintage, asymmetric |
| **TAPE** | Ampex/Studer oxide | Smooth, cohesive, natural |
| **FET** | 2N3819 JFET | Aggressive, fast, gritty |
| **IRON** | Output transformer | Subtle, transparent, 3D |

### Compressors (5 Models)

| Model | Based On | Character |
|-------|----------|-----------|
| **SSL Bus** | SSL G-Series VCA | Punchy glue, modern |
| **Fairchild 670** | Variable-mu tube | Smoothest ever made |
| **LA-2A** | T4B electro-optical | Transparent, musical |
| **1176** | UREI FET | Fastest, aggressive |
| **API 2500** | VCA + Thrust HPF | Dense, bass-preserving |

### Equalizers (5 Models)

| Model | Based On | Character |
|-------|----------|-----------|
| **Neve 1073** | Inductor + Class A | Musical warmth, bloom |
| **Neve 1084** | Extended 1073 | More flexible, surgical |
| **SSL 4000E** | State Variable Filter | Clean, precise, transparent |
| **Pultec EQP-1A** | Passive LC + tube | Phase bloom, vintage air |
| **API 550A** | Discrete Class A | Aggressive, focused |

\newpage

# Installation

## Method 1 — Installer (Recommended)

1. Double-click `ModulatedStrip_Setup_v1.0.0.exe`
2. Click through the wizard (requires administrator rights)
3. Click **Install**
4. Open **Ableton Live**
5. Go to **Options → Preferences → Plugins**
6. Click **Rescan Plugins**
7. Find **MODULATED STRIP** in your plugin browser
8. Drop it on any audio track

## Method 2 — Manual Install

1. Copy the `Modulated Strip.vst3` folder to:
C:\Program Files\Common Files\VST3\

text


2. Restart Ableton or rescan plugins

## Uninstall

1. Go to **Windows Settings → Apps**
2. Find **Modulated Strip**
3. Click **Uninstall**

> **Note:** Your user presets in `AppData\ModulatedStrip` are preserved after uninstall.

## System Requirements

| Requirement | Minimum |
|-------------|---------|
| OS | Windows 10/11 (64-bit) |
| CPU | Intel i5 / AMD Ryzen 5 |
| RAM | 4 GB |
| DAW | Ableton Live 11 or 12 |
| Format | VST3 |

\newpage

# Quick Start — 5 Minutes

## Hear It Working Immediately

1. Drop **Modulated Strip** on a drum loop track
2. Click the **preset name** in the plugin header
3. Select **"Deep House Bus"** or **"Drum Bus Punch"**
4. Press **Play**

**You should hear:**

- Drums sound thicker and more present
- More punch and definition
- Bass tighter and warmer
- Highs cleaner and airier

## Quick A/B Test

Toggle each **ON** button at the top of each section to hear what each processing stage adds independently.

| Button | What It Bypasses |
|--------|-----------------|
| SAT **ON** | Saturation stage |
| COMP **ON** | Compressor stage |
| EQ **ON** | Equalizer stage |

\newpage

# The Signal Chain

## Signal Flow Diagram
AUDIO INPUT
│
▼
┌────────────┐
│ INPUT GAIN │ -24 to +24 dB
└─────┬──────┘
▼
┌────────────┐
│ SATURATION │ Pick 1 of 7 circuits
│ STAGE │ Drive + Mix controls
└─────┬──────┘
▼
[EQ PRE switch determines order here]
▼
┌────────────┐
│ COMPRESSOR │ Pick 1 of 5 models
│ STAGE │ Full dynamics control
└─────┬──────┘
▼
┌────────────┐
│ EQ │ Pick 1 of 5 models
│ STAGE │ 3 bands + HPF
└─────┬──────┘
▼
┌────────────┐
│OUTPUT GAIN │ -24 to +24 dB
│ SOFT CLIP │ Protection at -0.5dBFS
└─────┬──────┘
▼
AUDIO OUTPUT

text


## EQ Pre/Post Switch

| Mode | Signal Path | Use Case |
|------|-------------|----------|
| **POST** (default) | SAT → COMP → EQ | Classic analog desk order. EQ shapes the compressed sound. Most predictable. |
| **PRE** | SAT → EQ → COMP | EQ shapes what the compressor reacts to. Boosting bass makes compressor work harder on bass. Advanced technique. |

> **Tip:** EQ PRE with a bass boost creates frequency-selective compression — a classic dub and bass music technique.

\newpage

# Saturation Models — Detailed Reference

## NEVE — Transformer + Class A

**Based on:** Neve 1073/1084 input section (Marinair transformer + 2520 op-amp)

**What it does to your signal:**

- Adds **even harmonics** (2nd, 4th) — the "warm" frequencies
- Slight **low-end bloom** from transformer core saturation
- **High frequencies gently softened** at higher signal levels
- DC offset from asymmetric clipping (handled by DC blocker)

**Harmonic profile:**
Fundamental │ ████████████████████
2nd harmonic│ ████████████ ← dominant (warmth)
3rd harmonic│ ████████
4th harmonic│ ████ ← secondary even
5th harmonic│ ███

text


**Best for:** Synth pads, bass, chord stabs, full mix bus

**Recommended settings:**

| Parameter | Value | Why |
|-----------|-------|-----|
| Drive | 15–25% | Subtle but transformative warmth |
| Mix | 75–90% | High wet — the character is gentle |

---

## SSL — VCA

**Based on:** SSL console VCA chip (THAT Corporation 2181)

**What it does to your signal:**

- Adds **odd harmonics** (3rd, 5th) — tight and clean
- **Symmetric** saturation — no DC offset
- Very subtle — "you feel it more than hear it"

**Harmonic profile:**
Fundamental │ ████████████████████
3rd harmonic│ ██ ← dominant (clarity)
5th harmonic│ █ ← barely measurable

text


**Best for:** Drums, percussion, modern electronic music

**Recommended settings:**

| Parameter | Value | Why |
|-----------|-------|-----|
| Drive | 8–20% | Barely audible but adds life |
| Mix | 60–80% | Toggle bypass to hear the difference |

---

## API — Discrete Op-Amp

**Based on:** API 2520 discrete op-amp module

**What it does to your signal:**

- **Linear below 60%** of input level — passes through clean
- **Hard knee above 60%** — sudden onset of saturation
- Mix of even and odd harmonics
- Forward, aggressive character

**Transfer curve:**
Output │ ●────── Hard knee region
│ ●──●╱
│ ●──●╱
│ ●─╱
│●╱ ← Clean linear region
└────────────────────── Input

text


**Best for:** Lead synths, arpeggios, anything needing to cut through

**Recommended settings:**

| Parameter | Value | Why |
|-----------|-------|-----|
| Drive | 25–40% | Forward presence and punch |
| Mix | 70–85% | Strong character, high wet |

---

## TUBE — 12AX7 Triode

**Based on:** 12AX7 vacuum tube (Child-Langmuir 3/2 power law)

**What it does to your signal:**

- **Asymmetric** clipping — positive and negative halves behave differently
- Strong **2nd harmonic** — the classic "tube warmth"
- Soft ceiling (plate saturation) but hard floor (tube cutoff at -0.8V)

**Transfer curve:**
Normal wave: After tube:
● ●─● ← soft rounded top
● ● ● ●
● ● ● ●
● ● ● ●

● ● ● ●
● ● ● ●
● ● ● ●
● ● ●─────● ← hard cutoff floor
●

text


**Best for:** Vocals, acoustic sounds, chord stabs, bass with richness

**Recommended settings:**

| Parameter | Value | Why |
|-----------|-------|-----|
| Drive | 15–30% | Warm vintage character |
| Mix | 75–90% | Watch asymmetry at high drive |

---

## TAPE — Magnetic Oxide

**Based on:** Ampex/Studer tape machine (arctangent transfer function)

**What it does to your signal:**

- **Symmetric** saturation — equal positive and negative
- Soft ceiling — signals never hard clip
- **Natural high-frequency compression** at high levels
- Cohesive "glue" effect on full mix

**Best for:** Full mix glue, drum buses, when things need to "sit together"

**Recommended settings:**

| Parameter | Value | Why |
|-----------|-------|-----|
| Drive | 10–20% | Cohesive glue |
| Mix | 65–80% | Lower for subtle, higher for obvious |

---

## FET — JFET Transistor

**Based on:** 2N3819 JFET input stage (same as 1176 compressor)

**What it does to your signal:**

- **Hard knee** at 70% of input level
- Clean below threshold, clips hard above
- Odd harmonics, gritty character
- Excellent **transient shaping**

**Best for:** Kicks, snares, drum buses, parallel saturation

**Recommended settings:**

| Parameter | Value | Why |
|-----------|-------|-----|
| Drive | 30–50% | Aggressive punch on transients |
| Mix | 40–60% | **Parallel is key** — high drive, lower mix |

---

## IRON — Output Transformer

**Based on:** Output transformer core saturation only (no amplifier stage)

**What it does to your signal:**

- Very **subtle** coloration
- Slight low-end bloom from inductance
- High frequencies gently softened
- The most transparent model

**Best for:** Mastering, final output stage, when you want minimal color

**Recommended settings:**

| Parameter | Value | Why |
|-----------|-------|-----|
| Drive | 5–15% | Intentionally minimal effect |
| Mix | 80–100% | Full wet — the effect is gentle |

\newpage

# Compressor Models — Detailed Reference

## SSL G-Series Bus Compressor

| Specification | Value |
|---------------|-------|
| **Circuit** | THAT2181 VCA |
| **Detection** | RMS (averaged, not peak) |
| **Sidechain HPF** | 30Hz built-in |
| **Auto-release** | Binary switch at 160ms |
| **Knee** | Hard (default) |

### RMS vs Peak Detection
PEAK detection: Responds to individual transients
Catches every spike
Can sound aggressive

RMS detection: Responds to average energy
(SSL uses this) Like how your ears hear loudness
Feels more natural and musical

text


### The SSL Auto-Release

The SSL has **two capacitors** in its release circuit:
0–160ms of compression: Normal user release time
After 160ms continuously: Fast cap takes over (~40ms)
Prevents low-frequency pumping

text


This binary switching creates the SSL "musical" release behavior.

### The Classic SSL Setting

| Parameter | Value |
|-----------|-------|
| Threshold | -15 to -20 dB |
| Ratio | 2:1 or 4:1 |
| Attack | 1–3 ms |
| Release | Maximum (auto release takes over) |
| Makeup | +1 to +2 dB |
| Mix | 60–80% |

> **Result:** Everything "glues" together. You barely hear it but you miss it when it's off.

---

## Fairchild 670

| Specification | Value |
|---------------|-------|
| **Circuit** | Variable-mu tube (vari-mu) |
| **Year** | 1959 |
| **Units made** | ~70 total |
| **Cost (used)** | $30,000 – $150,000 |
| **Tubes** | 14 per unit |
| **Used on** | Every Beatles record (Abbey Road's unit) |

### How Variable-Mu Works
Standard VCA: Uses a chip to control gain
Fast, precise, controllable

Variable-Mu: Uses tubes where the gain factor (mu)
(Fairchild) changes with DC bias current
The tube's own physics controls gain
Cannot be instant — naturally smooth

text


### The 6 Time Constants

The Fairchild has **no continuous attack/release knobs**. Instead it has a **6-position rotary switch**:

| Position | Attack | Release | Type |
|----------|--------|---------|------|
| TC 1 | 0.2 ms | 0.3 s | Fixed |
| TC 2 | 0.2 ms | 0.8 s | Fixed |
| TC 3 | 0.4 ms | 2.0 s | Fixed |
| **TC 4** | **0.4 ms** | **Auto** | **Program dependent** |
| TC 5 | 0.4 ms | 5.0 s | Fixed |
| TC 6 | 0.4 ms | Auto fast | Program dependent |

> **Start with TC 4** — the program-dependent mode where release time adapts to the music. This is the most musical setting.

| Parameter | Value |
|-----------|-------|
| Time Constant | TC 4 |
| Threshold | -20 to -22 dB |
| Mix | 60–70% |

---

## LA-2A Leveling Amplifier

| Specification | Value |
|---------------|-------|
| **Circuit** | T4B electro-optical cell |
| **Year** | 1965 |
| **Cost (used)** | $14,000 – $20,000 |
| **Controls** | Only two knobs |

### How The Opto Cell Works
┌──────────────┐ ┌────────────────┐
│ Electrolum- │ Light │ CdS Photo- │
│ escent Panel │ ──────► │ resistor │
│ (brightness │ │ (resistance │
│ = signal) │ │ = gain) │
└──────────────┘ └────────────────┘

text


**Light bulbs have inertia** — they cannot turn on/off instantly. This creates the natural attack time (~10–20ms).

**CdS cells have photon memory** — they stay conductive after the light goes off. This creates the **two-stage release**:

| Stage | Time | Cause |
|-------|------|-------|
| Fast release | ~40ms | Light fades (phosphor decay) |
| Slow release | 500ms–5s | CdS memory (photon persistence) |

### Controls

| Control | Note |
|---------|------|
| Attack | **FIXED** — optical physics only |
| COMPRESS / LIMIT | 3:1 ratio or 10:1 ratio |
| Peak Reduction | Threshold + ratio combined |
| Gain | Makeup gain |

| Parameter | Value |
|-----------|-------|
| Mode | COMPRESS |
| Peak Reduction | 30–50% |
| Mix | 80–100% |

---

## 1176 FET Compressor

| Specification | Value |
|---------------|-------|
| **Circuit** | 2N3819 JFET |
| **Year** | 1967 |
| **Attack range** | 0.02 ms – 0.8 ms (extremely fast) |

### ⚠ CONTROLS RUN BACKWARDS

On the real 1176, the attack and release knobs are **reversed** from every other compressor:

- Fully clockwise = **FASTEST** (not slowest)
- The **ATK ←** label indicates this

### Ratio Positions

| Ratio | Character |
|-------|-----------|
| 4:1 | Light, punchy, natural |
| 8:1 | Medium, controlled |
| 12:1 | Heavy, aggressive |
| 20:1 | Limiting |
| **ALL IN** | All four simultaneously — see below |

### ALL BUTTONS IN Mode

Pressing **ALL IN** engages all ratio buttons simultaneously. This is a **hardware anomaly** — you are "not supposed to" do this.

**But it sounds incredible:**

- Ratio becomes ~12:1 with a soft knee that **hardens** as compression increases
- Strong **FET saturation** is added
- Enormous **pumping punch** on drums

> **This is the sound of thousands of records.** Use at 30–50% mix for parallel compression.

---

## API 2500 Bus Compressor

| Specification | Value |
|---------------|-------|
| **Circuit** | THAT 2180 VCA |
| **Special** | Thrust circuit + topology switch |

### The THRUST Circuit
WITHOUT THRUST: WITH THRUST:
Compressor listens 150Hz HPF on sidechain
to ALL frequencies Bass does NOT trigger comp

Kick hits → comp reacts Kick hits → comp barely moves
Mix pumps with every kick Bass stays open and punchy

text


> **THRUST is why the API 2500 is popular in electronic music.** The kick and bass are what you want LOUD. THRUST keeps them loud while controlling everything else.

### Feed-Forward vs Feed-Back

| Mode | Sidechain reads | Character |
|------|----------------|-----------|
| FWD | Input signal | Faster, more aggressive |
| FBK | Output signal | More musical, forgiving |

### Classic Setting for Electronic Music

| Parameter | Value |
|-----------|-------|
| THRUST | ON |
| Topology | FWD |
| Threshold | -18 dB |
| Ratio | 4:1 |
| Attack | 0.5 ms |
| Release | 200 ms |
| Mix | 70% |

\newpage

# Equalizer Models — Detailed Reference

## Neve 1073

| Specification | Value |
|---------------|-------|
| **Circuit** | Passive inductor + 2520 Class A |
| **Low shelf** | **Boost only** (hardware constraint) |
| **Mid Q** | **Proportional** (not user adjustable) |
| **HPF** | 6 dB/octave (gentle slope) |

### Proportional Q — Why It Sounds Musical
Small boost (+3dB): Wide, gentle, transparent
──────────────┐
└──────────

Large boost (+12dB): Narrow, focused, precise
──────┐
└──────

The bandwidth self-adjusts to be appropriate
for the amount of boost. This is why Neve EQ
always sounds right no matter what you do.

text


### Inductor Bloom

When you boost the low shelf above +4 dB, a **subtle resonant peak** appears just above the shelf frequency. This is caused by the physical inductor in the circuit and creates the signature Neve "three-dimensional" bass quality.

### Classic Setting

| Band | Value | Purpose |
|------|-------|---------|
| Low boost | +4 dB at 60 Hz | Warm full bass |
| Mid | -2 dB at 350 Hz | Remove "cardboard" |
| High | +3 dB at 12 kHz | Silky air |

---

## SSL 4000E

| Specification | Value |
|---------------|-------|
| **Circuit** | State Variable Filter (SVF) for mid band |
| **Mid Q** | **Constant** — fully user controllable |
| **HPF** | 18 dB/octave (steep) |
| **Character** | Clean, precise, transparent |

### SVF vs Biquad

The SSL mid band uses a fundamentally different filter topology:

| | Biquad (Neve, API) | SVF (SSL) |
|---|---|---|
| Boost shape | Symmetric | **Slightly wider** |
| Cut shape | Symmetric | **Slightly narrower** |
| Sound | Focused | **Open and wide** |

> **This asymmetry is the SSL sound.** Boosts feel wide and open. Cuts feel surgical and precise.

---

## Pultec EQP-1A

| Specification | Value |
|---------------|-------|
| **Circuit** | Passive LC network + 12AU7 tube |
| **Year** | 1951 |
| **Cost (used)** | $15,000 – $30,000 |
| **HPF** | **None** (no high pass filter) |

### The Pultec Trick — The Most Famous EQ Setting

On the real Pultec, you can **boost and cut the same low frequency simultaneously**. This seems like it should cancel out. Instead it creates something magical:
Frequency response with +6dB at 60Hz:

Level │ ●●
│ ● ● ← Boost peak
│ ● ●
│ ● ●───────── ← Flat plateau
│──● ●
│ ● ← Controlled dip at 85Hz
└──────────────────────── Hz
20 40 60 80 100 200

text


**Result:**

| What happens | What you hear |
|-------------|---------------|
| Strong boost below 65 Hz | BIG sub bass |
| Tight definition at 70–80 Hz | PUNCHY attack |
| Controlled dip at 85 Hz | CLEAN transition to mids |

> **Try this on every kick drum and bass in your set.** Set LF boost to +6 to +8 dB at 60 Hz.

---

## API 550A

| Specification | Value |
|---------------|-------|
| **Circuit** | API 2520 discrete Class A op-amp |
| **Q type** | **Proportional — most aggressive** of all models |

### API Q vs Neve Q at Same Gain

| Boost | Neve Q | API Q |
|-------|--------|-------|
| +2 dB | 0.5 (very wide) | 0.7 (wide) |
| +6 dB | 0.7 | 0.9 |
| +12 dB | 1.3 | **1.4** (noticeably focused) |
| +15 dB | 2.0 | **2.5** (very narrow and incisive) |

> API gets narrow **much faster** than Neve. This creates the "incisive" API character — at high gain it drills into a very specific frequency.

### Output Coloration

The API adds both **2nd and 3rd harmonics** from the 2520 op-amp. The 3rd harmonic (edge/presence) is stronger than in Neve designs. This is why API sounds more forward and aggressive even when all gains are flat.

\newpage

# Features Reference

## Footer Controls

| Control | Function |
|---------|----------|
| **STEREO** | Stereo (linked), Mid/Side, or Dual Mono processing |
| **OVERSAMPLE** | 1x (off), 2x, 4x, 8x — quality selector for saturation |
| **DELTA** | Hear only what the plugin adds (subtracts dry signal) |
| **ANALOG** | Bypass all processing, only transformer color remains |
| **XTALK** | Inter-channel crosstalk (0.2%) — stereo hardware glue |
| **NOISE** | Model-specific analog noise floor (-100 to -110 dBFS) |

## Header Controls

| Control | Function |
|---------|----------|
| **< NAME >** | Preset navigation — click name to open browser |
| **STORE A / B** | Save current state to A or B comparison slot |
| **A / B** | Switch between stored states instantly |

## Meters

| Meter | What It Shows |
|-------|--------------|
| **Input LED meter** | Signal level entering the plugin |
| **Output LED meter** | Signal level after all processing |
| **Input CLIP LED** | Latches red when over -0.5 dBFS (click to clear) |
| **Output CLIP LED** | Same for output |
| **GR Needle** | Gain reduction in dB with physics-based needle |
| **GR Trace** | 2-second history of gain reduction (behind needle) |
| **EQ Curve** | Real-time frequency response of current EQ settings |
| **LUFS** | Short-term loudness in LUFS (3-second integration) |

\newpage

# Famous Signal Chain Combinations

## The Neve Stack — Classic Analog Richness

| Stage | Setting |
|-------|---------|
| **Saturation** | NEVE, Drive 20%, Mix 85% |
| **Compressor** | Fairchild TC3, -20dB, Mix 70% |
| **EQ** | Neve 1073: Low +3 at 80Hz, Mid -1 at 350Hz, High +2 at 12kHz |

**Sound:** Thick, warm, three-dimensional. Everything sounds like a million-dollar room.

**Use on:** Full mix bus, synth pads, chord stabs

---

## The SSL Glue — Modern Control

| Stage | Setting |
|-------|---------|
| **Saturation** | SSL, Drive 10%, Mix 70% |
| **Compressor** | SSL Bus, 4:1, -18dB, 3ms, auto release, Mix 70% |
| **EQ** | SSL 4000E: Low +1.5 at 80Hz, High +2 at 16kHz |

**Sound:** Controlled, open, polished. Everything sits together precisely.

**Use on:** Mix bus, drum bus, live performance output

---

## The Pultec Bass Trick — Most Famous EQ Setting

| Stage | Setting |
|-------|---------|
| **Saturation** | TUBE, Drive 15%, Mix 75% |
| **Compressor** | LA-2A, COMPRESS mode, 35%, Mix 90% |
| **EQ** | Pultec: LF boost +6dB at 60Hz (auto-cut at 85Hz), HF +3dB at 16kHz |

**Sound:** BIG sub + TIGHT punch + OPEN top. The classic kick/bass treatment.

**Use on:** Kick drums, bass, 808s

---

## The 1176 Parallel Trick — Drum Bus Density

| Stage | Setting |
|-------|---------|
| **Saturation** | FET, Drive 35%, Mix 60% |
| **Compressor** | 1176, 8:1, fast attack, 80ms release, **Mix 40%** |
| **EQ** | API 550A: Low +3 at 80Hz, Mid +2 at 3kHz, High +2 at 8kHz |

**Sound:** Punch, density, excitement. Original dynamics preserved but with massive weight behind them.

**Use on:** Drum bus, kick parallel bus

---

## The Deep House Bus — Built For This Plugin

| Stage | Setting |
|-------|---------|
| **Saturation** | NEVE, Drive 15%, Mix 80% |
| **Compressor** | SSL Bus, 2:1, -20dB, 3ms, 400ms, Mix 65% |
| **EQ** | Neve 1073: Low +2 at 60Hz, Mid -1 at 350Hz, High +2.5 at 12kHz |

**Sound:** Warm, controlled, professional deep house. The "finished record" sound.

**Use on:** Deep house master bus

\newpage

# Live Performance Guide

## Recommended Setup

### Master Track
Ableton Master Channel
└── Modulated Strip
└── Preset: "Live Mix Bus" or "Live Club Sound"

text


### With Return Tracks (Advanced)
Return A: Parallel Compression
└── Modulated Strip
└── Preset: "Parallel Drums"
└── Send from: kick, drums

Return B: Vintage Color
└── Modulated Strip
└── Preset: "Melodic Emotion"
└── Send from: synths, pads

text


## CPU Usage Guide

| Settings | CPU per instance (44.1kHz, i7) |
|----------|-------------------------------|
| Light (low drive, gentle comp) | 1.5–2.5% |
| Medium | 3–4% |
| Heavy | 5–7% |
| Heavy + 4x oversample | 8–12% |

### Reducing CPU During Live Performance

- Use **SSL** or **IRON** saturation (lowest CPU)
- Use **SSL Bus** compressor (efficient)
- Stay at **1x or 2x** oversampling
- Avoid very high drive settings (>70%)
- If CPU spikes, bypass **saturation** first (largest saving)

## Model Switching Safety

All models can be switched during playback:

- ✅ No clicks or pops
- ✅ Compressor state resets cleanly
- ✅ Audio continues without interruption
- ✅ Safe to assign to MIDI controllers

## Live Specific Presets

| Preset | Set Phase |
|--------|-----------|
| Live Set Opener | Opening, gentle warmth |
| Live Warmup | Building energy early |
| Live Build | Mid-set energy push |
| Live Peak Time | Maximum energy |
| Live Transition | Between sections |
| Live Club Sound | Club PA optimization |
| Live After Hours | Post-peak intimate |
| Live Mix Bus | General master treatment |

\newpage

# Testing Checklist

## Sound Quality

- [ ] All 7 saturation models sound distinctly different
- [ ] All 5 compressors feel different in timing and character
- [ ] All 5 EQs sound different at the same settings
- [ ] Pultec trick (low boost) sounds bigger AND tighter
- [ ] Fairchild feels musical and smooth, not aggressive
- [ ] 1176 feels fast and punchy
- [ ] LA-2A feels smooth and transparent
- [ ] SSL bus comp glues the mix together
- [ ] API 2500 with THRUST keeps bass open while controlling mids

## Feature Behavior

- [ ] Switch models during playback — no clicks
- [ ] Bypass each section — clean transition
- [ ] EQ PRE switch audibly changes the sound
- [ ] Delta button shows only the difference
- [ ] Analog button has only transformer color
- [ ] Stereo modes work (linked / M/S / dual mono)
- [ ] Oversampling improves quality at high drive
- [ ] Crosstalk is subtle but audible on stereo content
- [ ] Noise floor is barely audible (this is correct)
- [ ] Clip LEDs latch red and clear on click
- [ ] LUFS meter shows realistic values
- [ ] EQ curve display updates as knobs are moved
- [ ] GR needle meter moves with compression
- [ ] GR trace shows 2-second compression history

## Presets

- [ ] All 60+ factory presets load without error
- [ ] User preset save works
- [ ] User preset load restores all settings
- [ ] User preset delete works
- [ ] A/B comparison stores and recalls states
- [ ] Preset navigation arrows cycle correctly

## Stability

- [ ] Plays for 30+ minutes without issues
- [ ] Save and reload Ableton project — settings preserved
- [ ] Works at 44100, 48000, and 96000 Hz
- [ ] Works at buffer sizes 64, 128, 256, 512
- [ ] Multiple instances run simultaneously
- [ ] Close Ableton with plugin open — no crash
- [ ] Open and close plugin window repeatedly — no crash

## Live Performance

- [ ] Use in an actual live set or rehearsal
- [ ] CPU stays manageable throughout
- [ ] No audio dropouts during model switches
- [ ] Presets recall correctly under performance conditions
- [ ] Sound translates well on a real PA system

\newpage

# How to Report Feedback

## What to Include
WHAT happened:
WHAT you were doing:
WHICH preset/model/settings:
Ableton version:
Windows version:
CPU model:
Sample rate:
Buffer size:

text


## Priority of What We Need

### Critical (Report Immediately)

- Crashes or freezes
- Audio dropouts or silence
- Project save/load failures
- Plugin fails to load after install

### Important

- Models that sound wrong or identical
- Presets that don't match descriptions
- CPU usage that seems too high
- Visual glitches or text overlap

### Nice to Report

- Preset requests for specific genres
- Feature ideas for future versions
- Workflow improvement suggestions
- Comparison observations with real hardware

## Where to Send
Email: [your email]
Discord: [your discord]

Subject line format:
[v1.0.0] Brief description

Examples:
[v1.0.0] CRASH - session close with Fairchild selected
[v1.0.0] LA-2A compress/limit sounds identical
[v1.0.0] Request: preset for trance lead synth

text


\newpage

# Known Limitations (v1.0.0)

| Category | Limitation | Status |
|----------|-----------|--------|
| **UI** | Textures are programmatically generated (not custom artwork) | By design for v1.0 |
| **UI** | Window is not resizable | Planned for v1.1 |
| **CPU** | Oversampling 4x/8x is expensive | Use only for renders |
| **Features** | No MIDI learn | Planned for v1.1 |
| **Features** | No macro knobs | Planned for v1.1 |
| **Features** | No spectrum analyzer | Planned for v1.1 |
| **Features** | No skin/theme system | Planned for v1.2 |
| **Features** | M/S mode has no independent gain control | Planned for v1.1 |

\newpage

# Tips and Tricks

## Best Practices

1. **Start with presets** — always begin from a preset close to your goal, then tweak
2. **Use Delta mode** — toggle to hear ONLY what the plugin adds. Should be subtle.
3. **Parallel is your friend** — set drive HIGH, then lower MIX. Get character without losing dynamics.
4. **Calibrate with bypass** — use per-section ON buttons constantly
5. **Watch the GR meter** — 3–6 dB of gain reduction is the sweet spot for most uses
6. **Save your favorites** — when you find a great sound, save as user preset immediately

## Common Mistakes

| Mistake | Fix |
|---------|-----|
| Too much drive (>60%) | Most uses: 15–35% |
| Too much compression (high ratio + fast attack) | Start with 2:1 or 4:1 |
| Wrong model for the job | Match model to material character |
| Not using MIX knobs | 60–80% mix usually sounds better than 100% |
| Ignoring the meters | GR meter and LUFS are your guides |

\newpage

# Technical Specifications

| Spec | Value |
|------|-------|
| **Format** | VST3 |
| **Channels** | Stereo (Linked, M/S, Dual Mono) |
| **Sample rates** | 44.1k, 48k, 88.2k, 96k, 192k Hz |
| **Bit depth** | 32-bit float internal processing |
| **Oversampling** | 1x, 2x, 4x, 8x |
| **Latency** | 0 samples (1x) to ~256 samples (8x) |
| **Window size** | 1280 × 660 pixels |
| **Presets** | 60+ factory, unlimited user |
| **State save** | Full APVTS via XML in DAW project |
| **Soft clipper** | Transparent tanh at -0.5 dBFS |
| **Thread safety** | Atomic parameters, smoothed gains, no RT allocation |
| **DSP accuracy** | 94/100 per independent review |

---

*Modulated Strip VST — v1.0.0 Beta*
*Built with JUCE Framework*
*Tested on Ableton Live 11 and 12*
*Windows 10 and 11 (x64)*

**For beta testing purposes only. Do not redistribute.**