Exported labels are prefixed with a single underscore to avoid conflict with assembly labels.
Static labels are prefixed with two underscores to avoid conflicts with exported labels.

Initialized global variables will add to two RGBASM `SECTION FRAGMENT`s, one in
ROMX and the other in WRAM0. Before jumping to main the ROMX section will be
copied to the WRAM section, setting them to the pre-defined values.