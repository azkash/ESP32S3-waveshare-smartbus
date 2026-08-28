---
name: Cyber-Industrial Interface
colors:
  surface: '#0b1326'
  surface-dim: '#0b1326'
  surface-bright: '#31394d'
  surface-container-lowest: '#060e20'
  surface-container-low: '#131b2e'
  surface-container: '#171f33'
  surface-container-high: '#222a3d'
  surface-container-highest: '#2d3449'
  on-surface: '#dae2fd'
  on-surface-variant: '#c2c6d6'
  inverse-surface: '#dae2fd'
  inverse-on-surface: '#283044'
  outline: '#8c909f'
  outline-variant: '#424754'
  surface-tint: '#adc6ff'
  primary: '#adc6ff'
  on-primary: '#002e6a'
  primary-container: '#4d8eff'
  on-primary-container: '#00285d'
  inverse-primary: '#005ac2'
  secondary: '#4edea3'
  on-secondary: '#003824'
  secondary-container: '#00a572'
  on-secondary-container: '#00311f'
  tertiary: '#ffb3ad'
  on-tertiary: '#68000a'
  tertiary-container: '#ff5451'
  on-tertiary-container: '#5c0008'
  error: '#ffb4ab'
  on-error: '#690005'
  error-container: '#93000a'
  on-error-container: '#ffdad6'
  primary-fixed: '#d8e2ff'
  primary-fixed-dim: '#adc6ff'
  on-primary-fixed: '#001a42'
  on-primary-fixed-variant: '#004395'
  secondary-fixed: '#6ffbbe'
  secondary-fixed-dim: '#4edea3'
  on-secondary-fixed: '#002113'
  on-secondary-fixed-variant: '#005236'
  tertiary-fixed: '#ffdad7'
  tertiary-fixed-dim: '#ffb3ad'
  on-tertiary-fixed: '#410004'
  on-tertiary-fixed-variant: '#930013'
  background: '#0b1326'
  on-background: '#dae2fd'
  surface-variant: '#2d3449'
typography:
  display-metric:
    fontFamily: JetBrains Mono
    fontSize: 48px
    fontWeight: '700'
    lineHeight: 48px
    letterSpacing: -0.02em
  headline-lg:
    fontFamily: Hanken Grotesk
    fontSize: 24px
    fontWeight: '600'
    lineHeight: 32px
  headline-md:
    fontFamily: Hanken Grotesk
    fontSize: 18px
    fontWeight: '600'
    lineHeight: 24px
  body-sm:
    fontFamily: Hanken Grotesk
    fontSize: 14px
    fontWeight: '400'
    lineHeight: 20px
  label-caps:
    fontFamily: Hanken Grotesk
    fontSize: 12px
    fontWeight: '700'
    lineHeight: 16px
    letterSpacing: 0.05em
  data-mono:
    fontFamily: JetBrains Mono
    fontSize: 16px
    fontWeight: '500'
    lineHeight: 16px
rounded:
  sm: 0.125rem
  DEFAULT: 0.25rem
  md: 0.375rem
  lg: 0.5rem
  xl: 0.75rem
  full: 9999px
spacing:
  unit: 4px
  gutter: 12px
  margin: 16px
  container-padding: 12px
---

## Brand & Style

This design system is engineered for high-performance hardware monitoring and industrial automation. The aesthetic is "Modern Industrial"—a fusion of functional minimalism and high-tech utility. It prioritizes rapid information processing and situational awareness.

The visual language draws from **Modernism** and **Technical Minimalist** movements, utilizing heavy whitespace (within the constraints of a small screen) to separate critical data streams. To reflect the "rugged" nature of ESP32 hardware, the design incorporates subtle glows for active states, evoking the feel of high-end rack-mounted equipment or aeronautical instrumentation. The interface is optimized for an 800x480 resolution, ensuring every pixel serves a functional purpose.

## Colors

The palette is anchored in a deep, nocturnal background to reduce eye strain in low-light industrial environments.

- **Primary (Electric Blue):** Used for standard data telemetry, active toggles, and primary navigation focus.
- **Success (Green):** Indicates nominal operating ranges and completed processes.
- **Danger (Red):** Reserved strictly for critical alerts, hardware failures, or values exceeding safety thresholds.
- **Neutral/Surface:** Shades of slate and navy are used to create structural depth and grouping without distracting from the data.

Active elements utilize a "glow" treatment: a low-opacity outer glow using the primary color to simulate an illuminated physical indicator.

## Typography

This system employs a dual-font strategy to balance legibility with technical precision.

1.  **Monospaced (JetBrains Mono):** Used for all numerical values, timestamps, and status codes. This ensures that changing digits do not cause "jitter" in the UI and that columns of numbers align perfectly.
2.  **Sans-Serif (Hanken Grotesk):** Used for labels, descriptions, and structural headers. It provides a clean, modern contrast to the technical monospaced data.

On the 800x480 display, avoid font sizes smaller than 12px to maintain readability at arm's length. Use `label-caps` for all non-interactive descriptive labels to differentiate them from actionable text.

## Layout & Spacing

The layout follows a strict **12-column fluid grid** adjusted for the specific 5:3 aspect ratio of the 800x480 screen. 

- **The Dashboard Shell:** A 16px outer margin defines the safe area. 
- **The Module System:** Content is divided into "Modules" (Cards). Modules should span 3, 4, 6, or 12 columns.
- **Rhythm:** A 4px baseline grid governs all internal vertical spacing. Use 12px gutters between modules to create clear visual separation without wasting limited screen real estate.

The layout is optimized for a single-screen view with no scrolling; if more data is required, use a tabbed interface rather than vertical overflow.

## Elevation & Depth

To maintain an industrial feel, the design avoids soft, natural shadows in favor of **Tonal Layering** and **Technical Outlines**.

- **Level 0 (Background):** The darkest color (`#020617`), representing the base chassis.
- **Level 1 (Module Surface):** Elevated slightly using `#1e293b`. Modules use a 1px solid border (`#334155`) to define their boundaries.
- **Active State:** When a component or sensor is "active," it receives a primary color border and a subtle 4px blur glow (`rgba(59, 130, 246, 0.3)`).
- **Depth:** Use inner-shadows on input fields or "wells" to suggest they are recessed into the hardware panel.

## Shapes

The shape language is "Soft-Industrial." While sharp corners are common in industrial design, a slight radius (`4px`) is applied to all modules and buttons to improve touch-target perception and modern readability.

- **Modules/Cards:** 4px (`rounded-sm`).
- **Buttons:** 4px (`rounded-sm`).
- **Status Indicators:** Circular (100% radius) for small "LED" style pips.
- **Progress Bars:** Flat ends (0px) to emphasize the linear, technical nature of the measurement.

## Components

- **Buttons:** Should be rectangular with high-contrast labels. Primary buttons use a solid Electric Blue fill with white text. Secondary buttons use a ghost style (1px border) with no fill.
- **Data Cards:** Every card must have a `label-caps` header. The main metric should use the `display-metric` size in Monospace.
- **Status Pips:** Small circular indicators that mimic physical LEDs. Use Success Green for "Online," Danger Red for "Fault," and Slate for "Standby."
- **Input Fields:** Recessed styling with a 1px bottom border that lights up Electric Blue when focused. 
- **Gauges & Sparklines:** Use 2px stroke widths for sparklines. Gauges should be simplified (linear or semi-circular) with clear markings for Danger/Success zones.
- **Toggle Switches:** Square-ish design (rounded-sm) rather than pill-shaped to stay consistent with the industrial aesthetic.