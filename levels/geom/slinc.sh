##########################################
# SLINC.TR2 Geometry Optimisation Script #
##########################################

# Nothing to do here
#
# TRIED 2026-09-15: removing the fully isolated, unreachable room cluster
# 26/27/28 (no portal to/from any other room, no items, no camera inside).
# REVERTED -- rebuilding Reunion through the full pipeline (texmod/troi/
# tr3mesh) for the first time this session exposed a pre-existing, never-
# before-tested problem: Reunion's Lara-model-fix is baked directly into
# levels/in/slinc.tr2 and touches texture tile 11 -- the SAME tile texmod
# uses for slinc's own controller/savecrystal graphics (identical collision
# pattern to the Undersea/Underse2 tile-12 saga this session, just never
# triggered before because every prior Reunion fix was a byte-patch on the
# already-built .psx, not a fresh rebuild). Result: broken controller/
# crystal textures AND Sophia no longer dying (a separate regression, cause
# not yet root-caused). Not worth pursuing without solving the tile-11
# collision first -- same unresolved problem as Undersea, out of scope for
# a "remove 3 empty rooms" side quest.
