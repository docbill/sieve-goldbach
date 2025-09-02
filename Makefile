# Makefile - builds source
# Copyright (C) 2025 Bill C. Riemers
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# SPDX-License-Identifier: GPL-3.0-or-later

# Makefile — idempotent build/generate/certify/verify (DRY version)

SHELL := /bin/sh
.DELETE_ON_ERROR:
MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

OUT  := output
DATA := data

# Make one of these your default (pick what you want as the "do the thing" target)
.DEFAULT_GOAL := validate-skip-summary-ml
.PHONY: all
all: validate-skip-summary-ml

# ---------- Parameters ----------
LIMIT        := 200000000
GBCOUNT      := 10000
SMALLCOUNT   := 1000000
MEDIUMCOUNT  := 10000000
LARGECOUNT   := 100000000

# Size axes
SIZES      := SMALL MEDIUM LARGE
SUFFIX_SMALL  := 1M
SUFFIX_MEDIUM := 10M
SUFFIX_LARGE  := 100M
COUNT_SMALL   := $(SMALLCOUNT)
COUNT_MEDIUM  := $(MEDIUMCOUNT)
COUNT_LARGE   := $(LARGECOUNT)

# ---------- Binaries (real paths) ----------
PRIME_BITMAP_BIN := src/primesieve_bitmap/primesieve_bitmap
STOREPRIMES_BIN  := src/storeprimes/storeprimes
FINDGBPAIRS_BIN  := src/findgbpairs/findgbpairs
CPSLB_BIN        := src/cpslowerbound/cpslowerbound
SGB_BIN          := src/pairrange2sgbll/pairrange2sgbll
SUMMARY_BIN      := src/pairrangesummary/pairrangesummary
CERTIFYPRIMES    := src/certifyprimes/certifyprimes
CERTIFYGBPAIRS   := src/certifygbpairs/certifygbpairs
VALIDATESUMMARY  := src/validatepairrangesummary/validatepairrangesummary

PROGRAMS := $(PRIME_BITMAP_BIN) $(STOREPRIMES_BIN) $(FINDGBPAIRS_BIN) \
            $(CPSLB_BIN) $(SGB_BIN) $(CERTIFYPRIMES) $(CERTIFYGBPAIRS) \
            $(SUMMARY_BIN) $(VALIDATESUMMARY)

# Dispatcher: build any src/<prog>/<prog> via src/Makefile (inherits toolchain)
$(PROGRAMS):
	$(MAKE) -C src $(notdir $@)

# ---------- Base outputs (non-S/M/L) ----------
BITMAP_FILE := primes-200M.bitmap
RAW_FILE    := primes-200M.raw
GBP_FILE    := gbpairs-10000.csv

BITMAP := $(OUT)/$(BITMAP_FILE)
RAW    := $(OUT)/$(RAW_FILE)
GBP    := $(OUT)/$(GBP_FILE)

# Ensure output dir exists (order-only)
$(OUT):
	mkdir -p "$(OUT)"

# Generate base artifacts
$(BITMAP): $(PRIME_BITMAP_BIN) | $(OUT)
	./$(PRIME_BITMAP_BIN) $(LIMIT) > "$@"

$(RAW): $(STOREPRIMES_BIN) $(BITMAP) | $(OUT)
	./$(STOREPRIMES_BIN) "$(BITMAP)" "$@"

$(GBP): $(FINDGBPAIRS_BIN) $(RAW) | $(OUT)
	./$(FINDGBPAIRS_BIN) "$(RAW)" $(GBCOUNT) > "$@"

# ---------- Gold (reference) files ----------

BITMAP_GOLD := $(DATA)/$(notdir $(BITMAP)).verify
RAW_GOLD    := $(DATA)/$(notdir $(RAW)).verify
GBP_GOLD    := $(DATA)/$(notdir $(GBP)).verify

# ---------- Per-size templated outputs & verifies ----------
# Templated variables expanded for each SIZE in $(SIZES)
# For SIZE in {SMALL,MEDIUM,LARGE} this defines:
#   SGB_{SIZE}, SUMMARY_{SIZE}, CPSLB_{SIZE}, LAMBDA_{TYPE}_{SIZE}
#   their generation rules, and corresponding *_VERIFY targets.

# Helper to read “value of variable named NAME_SIZE”
# Usage: $(call GET,SUFFIX,SMALL) -> 1M
GET = $($(1)_$(2))

# ---------------------------------------------------------------------------

define SIZE_TEMPLATE

# File stems for $(1) = SIZE (e.g., SMALL)
SFX_$(1) := $(call GET,SUFFIX,$(1))
CNT_$(1) := $(call GET,COUNT,$(1))

SGB_FILE_$(1)       := pairrange2sgbll-$$(SFX_$(1)).csv
SUMMARY_FILE_$(1)   := pairrangesummary-$$(SFX_$(1)).csv
JOIN_FILE_$(1)      := pairrangejoin-$$(SFX_$(1)).csv
CPSLB_FILE_$(1)     := cpslowerbound-$$(SFX_$(1)).csv
LAVG_FILE_$(1)      := lambdaavg-$$(SFX_$(1)).csv
LMIN_FILE_$(1)      := lambdamin-$$(SFX_$(1)).csv
LMAX_FILE_$(1)      := lambdamax-$$(SFX_$(1)).csv
LSAVG_FILE_$(1)      := lambdastatsavg-$$(SFX_$(1)).csv
LSMIN_FILE_$(1)      := lambdastatsmin-$$(SFX_$(1)).csv
LSMAX_FILE_$(1)      := lambdastatsmax-$$(SFX_$(1)).csv

SGB_$(1)      := $(OUT)/$$(SGB_FILE_$(1))
SUMMARY_$(1)  := $(OUT)/$$(SUMMARY_FILE_$(1))
JOIN_$(1)     := $(OUT)/$$(JOIN_FILE_$(1))
CPSLB_$(1)    := $(OUT)/$$(CPSLB_FILE_$(1))
LAVG_$(1)     := $(OUT)/$$(LAVG_FILE_$(1))
LMIN_$(1)     := $(OUT)/$$(LMIN_FILE_$(1))
LMAX_$(1)     := $(OUT)/$$(LMAX_FILE_$(1))
LSAVG_$(1)     := $(OUT)/$$(LSAVG_FILE_$(1))
LSMIN_$(1)     := $(OUT)/$$(LSMIN_FILE_$(1))
LSMAX_$(1)     := $(OUT)/$$(LSMAX_FILE_$(1))

OUTPUT_$(1)   := $$(SGB_$(1)) $$(SUMMARY_$(1)) $$(JOIN_$(1)) $$(CPSLB_$(1)) \
	         $$(LAVG_$(1)) $$(LMIN_$(1)) $$(LMAX_$(1)) \
	         $$(LSAVG_$(1)) $$(LSMIN_$(1)) $$(LSMAX_$(1))

# Verifies (sha256 or tool-specific)
SGB_VERIFY_$(1)      := $$(SGB_$(1)).sha256
SUMMARY_VERIFY_$(1)  := $$(SUMMARY_$(1)).verify
JOIN_VERIFY_$(1)     := $$(JOIN_$(1)).sha256
CPSLB_VERIFY_$(1)    := $$(CPSLB_$(1)).sha256
LAVG_VERIFY_$(1)     := $$(LAVG_$(1)).sha256
LMIN_VERIFY_$(1)     := $$(LMIN_$(1)).sha256
LMAX_VERIFY_$(1)     := $$(LMAX_$(1)).sha256
LSAVG_VERIFY_$(1)     := $$(LSAVG_$(1)).sha256
LSMIN_VERIFY_$(1)     := $$(LSMIN_$(1)).sha256
LSMAX_VERIFY_$(1)     := $$(LSMAX_$(1)).sha256

# Corresponding Gold references
SGB_GOLD_$(1)     := $(DATA)/$$(SGB_FILE_$(1)).sha256
SUMMARY_GOLD_$(1) := $(DATA)/$$(SUMMARY_FILE_$(1)).verify
JOIN_GOLD_$(1)    := $(DATA)/$$(JOIN_FILE_$(1)).sha256
CPSLB_GOLD_$(1)   := $(DATA)/$$(CPSLB_FILE_$(1)).sha256
LAVG_GOLD_$(1)    := $(DATA)/$$(LAVG_FILE_$(1)).sha256
LMIN_GOLD_$(1)    := $(DATA)/$$(LMIN_FILE_$(1)).sha256
LMAX_GOLD_$(1)    := $(DATA)/$$(LMAX_FILE_$(1)).sha256
LSAVG_GOLD_$(1)    := $(DATA)/$$(LSAVG_FILE_$(1)).sha256
LSMIN_GOLD_$(1)    := $(DATA)/$$(LSMIN_FILE_$(1)).sha256
LSMAX_GOLD_$(1)    := $(DATA)/$$(LSMAX_FILE_$(1)).sha256

endef

# Expand the per-size template for all sizes
$(foreach SZ,$(SIZES),$(eval $(call SIZE_TEMPLATE,$(SZ))))

# --- Rebind only after all per-size vars are defined ---
ifeq ($(SKIP_SUMMARY_ML),1)
  SUMMARY_MEDIUM := $(DATA)/$(SUMMARY_FILE_MEDIUM)
  SUMMARY_LARGE  := $(DATA)/$(SUMMARY_FILE_LARGE)
  SUMMARY_VERIFY_MEDIUM := 
  SUMMARY_VERIFY_LARGE  := 
endif


define SIZE_TEMPLATE2

# --- Generation rules ---

# Predicted HL-A pairs
$$(SGB_$(1)): $(SGB_BIN) $(RAW) | $(OUT)
	$$(if $$(filter LARGE,$(1)), \
		./$(SGB_BIN) "$(RAW)" $$(CNT_$(1)) | tee "$$@", \
		$$(if $$(filter MEDIUM,$(1)), \
			( if [ -r "$$(call GET,SGB,LARGE)" ]; then grep -v '^7,'  < "$$(call GET,SGB,LARGE)"; else ./$(SGB_BIN) "$(RAW)" $$(CNT_$(1)); fi ) | tee "$$@", \
			( if [ -r "$$(call GET,SGB,LARGE)" ]; then grep -v '^[67],' < "$$(call GET,SGB,LARGE)"; elif [ -r "$$(call GET,SGB,MEDIUM)" ]; then grep -v '^6,' < "$$(call GET,SGB,MEDIUM)"; else ./$(SGB_BIN) "$(RAW)" $$(CNT_$(1)); fi ) | tee "$$@" \
		) \
	)

# Fast summary counts
# - If SKIP_SUMMARY_ML=1 and SIZE is MEDIUM or LARGE: DO NOT define a rule (read data/)
# - If SKIP_SUMMARY_ML=1 and SIZE is SMALL: always generate (no reuse)
# - Else: keep the existing reuse-from-larger behavior
ifeq ($(and $(SKIP_SUMMARY_ML),$(filter MEDIUM LARGE,$(1))),)
$$(SUMMARY_$(1)): $(SUMMARY_BIN) $(RAW) | $(OUT)
	$$(if $$(and $$(SKIP_SUMMARY_ML),$$(filter SMALL,$(1))), \
		./$(SUMMARY_BIN) "$(RAW)" $$(CNT_$(1)) | tee "$$@", \
		$$(if $$(filter LARGE,$(1)), \
			./$(SUMMARY_BIN) "$(RAW)" $$(CNT_$(1)) | tee "$$@", \
			$$(if $$(filter MEDIUM,$(1)), \
				( if [ -r "$$(call GET,SUMMARY,LARGE)" ]; then grep -v '^7,'  < "$$(call GET,SUMMARY,LARGE)"; else ./$(SUMMARY_BIN) "$(RAW)" $$(CNT_$(1)); fi ) | tee "$$@", \
				( if [ -r "$$(call GET,SUMMARY,LARGE)" ]; then grep -v '^[67],' < "$$(call GET,SUMMARY,LARGE)"; elif [ -r "$$(call GET,SUMMARY,MEDIUM)" ]; then grep -v '^6,' < "$$(call GET,SUMMARY,MEDIUM)"; else ./$(SUMMARY_BIN) "$(RAW)" $$(CNT_$(1)); fi ) | tee "$$@" \
			) \
		) \
	)
endif

# Join summary as sgb files
$$(JOIN_$(1)): $$(SUMMARY_$(1)) $$(SGB_$(1)) | $(OUT)
	chmod ugo+x ./bin/joinSumPred.awk
	./bin/joinSumPred.awk "$$(SUMMARY_$(1))" "$$(SGB_$(1))" > "$$@"

# CPS lower bound (needs n_0 from SUMMARY)
$$(CPSLB_$(1)): $(CPSLB_BIN) $(RAW) $$(SUMMARY_$(1)) | $(OUT)
	cut -d ',' -f6 < "$$(SUMMARY_$(1))" | tail -n +2 > "$(OUT)/n_0.txt"
	./$(CPSLB_BIN) "$(RAW)" "$(OUT)/n_0.txt" | tee "$$@"
	$(RM) "$(OUT)/n_0.txt"


# Lambda CSVs from compare* awk (shared)
$$(LAVG_$(1)): $$(SUMMARY_$(1)) $$(SGB_$(1)) | $(OUT)
	chmod ugo+x ./bin/compareAvg.awk
	./bin/compareAvg.awk "$$(SUMMARY_$(1))" "$$(SGB_$(1))" > "$$@"

$$(LMIN_$(1)): $$(SUMMARY_$(1)) $$(SGB_$(1)) | $(OUT)
	chmod ugo+x ./bin/compareMin.awk
	./bin/compareMin.awk "$$(SUMMARY_$(1))" "$$(SGB_$(1))" > "$$@"

$$(LMAX_$(1)): $$(SUMMARY_$(1)) $$(SGB_$(1)) | $(OUT)
	chmod ugo+x ./bin/compareMax.awk
	./bin/compareMax.awk "$$(SUMMARY_$(1))" "$$(SGB_$(1))" > "$$@"

# Lambda Stats CSVs from lambdaStats awk (shared)
$$(LSAVG_$(1)): $$(LAVG_$(1)) | $(OUT)
	chmod ugo+x ./bin/lambdaStats.awk
	./bin/lambdaStats.awk "$$(LAVG_$(1))" > "$$@"

$$(LSMIN_$(1)): $$(LMIN_$(1)) | $(OUT)
	chmod ugo+x ./bin/lambdaStats.awk
	./bin/lambdaStats.awk "$$(LMIN_$(1))" > "$$@"

$$(LSMAX_$(1)): $$(LMAX_$(1)) | $(OUT)
	chmod ugo+x ./bin/lambdaStats.awk
	./bin/lambdaStats.awk "$$(LMAX_$(1))" > "$$@"

# --- Verifications ---

# SUMMARY verify uses validator + SHA
ifeq ($(and $(SKIP_SUMMARY_ML),$(filter MEDIUM LARGE,$(1))),)
$$(SUMMARY_VERIFY_$(1)): $(VALIDATESUMMARY) $(BITMAP) $(RAW) $$(SUMMARY_$(1)) | $(OUT)
	$(VALIDATESUMMARY) --file "$$(SUMMARY_$(1))" --bitmap "$(BITMAP)" --raw "$(RAW)" | tee "$$@"
	sha256sum "$$(SUMMARY_$(1))" | tee -a "$$@"
endif

# sha256-only verifies use a single pattern rule (see below)
# We keep them here for dependency wiring:
$$(SGB_VERIFY_$(1)):    $$(SGB_$(1))

$$(JOIN_VERIFY_$(1)):    $$(JOIN_$(1))

$$(CPSLB_VERIFY_$(1)):  $$(CPSLB_$(1))

$$(LAVG_VERIFY_$(1)):   $$(LAVG_$(1))

$$(LMIN_VERIFY_$(1)):   $$(LMIN_$(1))

$$(LMAX_VERIFY_$(1)):   $$(LMAX_$(1))

$$(LSAVG_VERIFY_$(1)):   $$(LSAVG_$(1))

$$(LSMIN_VERIFY_$(1)):   $$(LSMIN_$(1))

$$(LSMAX_VERIFY_$(1)):   $$(LSMAX_$(1))

endef


# Expand the per-size template for all sizes
$(foreach SZ,$(SIZES),$(eval $(call SIZE_TEMPLATE2,$(SZ))))


# ---------- Generic sha256 rule ----------
%.sha256: %
	sha256sum "$<" | tee "$@"

# ---------- Top-level generation groups ----------
generate:       $(BITMAP) $(RAW) $(GBP) $(SGB_SMALL) $(SUMMARY_SMALL) \
		$(JOIN_SMALL) $(CPSLB_SMALL) $(LAVG_SMALL) $(LMIN_SMALL) \
		$(LMAX_SMALL) $(LSAVG_SMALL) $(LSMIN_SMALL) $(LSMAX_SMALL)

generate-medium: $(BITMAP) $(RAW) $(GBP) $(SGB_MEDIUM) $(SUMMARY_MEDIUM) \
		$(JOIN_MEDIUM) $(CPSLB_MEDIUM) $(LAVG_MEDIUM) $(LMIN_MEDIUM) \
		$(LMAX_MEDIUM) $(LSAVG_MEDIUM) $(LSMIN_MEDIUM) $(LSMAX_MEDIUM)

generate-large: $(BITMAP) $(RAW) $(GBP) $(SGB_LARGE) $(SUMMARY_LARGE) \
		$(JOIN_LARGE) $(CPSLB_LARGE) $(LAVG_LARGE) $(LMIN_LARGE) \
		$(LMAX_LARGE) $(LSAVG_LARGE) $(LSMIN_LARGE) $(LSMAX_LARGE)

# ---------- Certification (base + per-size chains) ----------
BITMAP_VERIFY := $(BITMAP).verify
RAW_VERIFY    := $(RAW).verify
GBP_VERIFY    := $(GBP).verify

$(BITMAP_VERIFY): $(CERTIFYPRIMES) $(BITMAP) | $(OUT)
	./$(CERTIFYPRIMES) --bitmap --file "$(BITMAP)" | tee "$@"

$(RAW_VERIFY): $(CERTIFYPRIMES) $(RAW) | $(OUT)
	./$(CERTIFYPRIMES) --binary --file "$(RAW)" | tee "$@"

$(GBP_VERIFY): $(CERTIFYGBPAIRS) $(GBP) $(BITMAP) | $(OUT)
	./$(CERTIFYGBPAIRS) --bitmap "$(BITMAP)" --file "$(GBP)" | tee "$@"

certify: $(BITMAP_VERIFY) $(RAW_VERIFY) $(GBP_VERIFY) $(SGB_VERIFY_SMALL) \
	$(SUMMARY_VERIFY_SMALL) $(JOIN_VERIFY_SMALL) $(CPSLB_VERIFY_SMALL) \
	$(LAVG_VERIFY_SMALL) $(LMIN_VERIFY_SMALL) $(LMAX_VERIFY_SMALL) \
        $(LSAVG_VERIFY_SMALL) $(LSMIN_VERIFY_SMALL) $(LSMAX_VERIFY_SMALL)

certify-medium: $(SGB_VERIFY_MEDIUM) $(SUMMARY_VERIFY_MEDIUM) \
	$(JOIN_VERIFY_MEDIUM) $(CPSLB_VERIFY_MEDIUM) \
	$(LAVG_VERIFY_MEDIUM) $(LMIN_VERIFY_MEDIUM) $(LMAX_VERIFY_MEDIUM) \
	$(LSAVG_VERIFY_MEDIUM) $(LSMIN_VERIFY_MEDIUM) $(LSMAX_VERIFY_MEDIUM) \
	certify

certify-large: $(SGB_VERIFY_LARGE) $(SUMMARY_VERIFY_LARGE) \
	$(JOIN_VERIFY_LARGE) $(CPSLB_VERIFY_LARGE) \
	$(LAVG_VERIFY_LARGE) $(LMIN_VERIFY_LARGE) $(LMAX_VERIFY_LARGE) \
	$(LSAVG_VERIFY_LARGE) $(LSMIN_VERIFY_LARGE) $(LSMAX_VERIFY_LARGE) \
	certify-medium

# ---------- Compare against golden references ----------
.PHONY: verify verify-medium verify-large validate validate-medium validate-large

verify: certify
	@cmp "$(BITMAP_VERIFY)" "$(BITMAP_GOLD)" && echo "Validated $(BITMAP)"
	@cmp "$(RAW_VERIFY)"    "$(RAW_GOLD)" && echo "Validated $(RAW)"
	@cmp "$(GBP_VERIFY)"    "$(GBP_GOLD)" && echo "Validated $(GBP)"
	@cmp "$(SGB_VERIFY_SMALL)"    "$(SGB_GOLD_SMALL)" && echo "Validated $(SGB_SMALL)"
	@cmp "$(SUMMARY_VERIFY_SMALL)" "$(SUMMARY_GOLD_SMALL)" && echo "Validated $(SUMMARY_SMALL)"
	@cmp "$(JOIN_VERIFY_SMALL)" "$(JOIN_GOLD_SMALL)" && echo "Validated $(JOIN_SMALL)"
	@cmp "$(CPSLB_VERIFY_SMALL)"   "$(CPSLB_GOLD_SMALL)" && echo "Validated $(CPSLB_SMALL)"
	@cmp "$(LAVG_VERIFY_SMALL)"    "$(LAVG_GOLD_SMALL)" && echo "Validated $(LAVG_SMALL)"
	@cmp "$(LMIN_VERIFY_SMALL)"    "$(LMIN_GOLD_SMALL)" && echo "Validated $(LMIN_SMALL)"
	@cmp "$(LMAX_VERIFY_SMALL)"    "$(LMAX_GOLD_SMALL)" && echo "Validated $(LMAX_SMALL)"
	@cmp "$(LSAVG_VERIFY_SMALL)"    "$(LSAVG_GOLD_SMALL)" && echo "Validated $(LSAVG_SMALL)"
	@cmp "$(LSMIN_VERIFY_SMALL)"    "$(LSMIN_GOLD_SMALL)" && echo "Validated $(LSMIN_SMALL)"
	@cmp "$(LSMAX_VERIFY_SMALL)"    "$(LSMAX_GOLD_SMALL)" && echo "Validated $(LSMAX_SMALL)"

verify-medium: certify-medium verify
	@cmp "$(SGB_VERIFY_MEDIUM)"    "$(SGB_GOLD_MEDIUM)" && echo "Validated $(SGB_MEDIUM)"
	@test -n "$(SKIP_SUMMARY_ML)" || (cmp "$(SUMMARY_VERIFY_MEDIUM)" "$(SUMMARY_GOLD_MEDIUM)" && echo "Validated $(SUMMARY_MEDIUM)")
	@cmp "$(JOIN_VERIFY_MEDIUM)"   "$(JOIN_GOLD_MEDIUM)" && echo "Validated $(JOIN_MEDIUM)"
	@cmp "$(CPSLB_VERIFY_MEDIUM)"   "$(CPSLB_GOLD_MEDIUM)" && echo "Validated $(CPSLB_MEDIUM)"
	@cmp "$(LAVG_VERIFY_MEDIUM)"    "$(LAVG_GOLD_MEDIUM)" && echo "Validated $(LAVG_MEDIUM)"
	@cmp "$(LMIN_VERIFY_MEDIUM)"    "$(LMIN_GOLD_MEDIUM)" && echo "Validated $(LMIN_MEDIUM)"
	@cmp "$(LMAX_VERIFY_MEDIUM)"    "$(LMAX_GOLD_MEDIUM)" && echo "Validated $(LMAX_MEDIUM)"
	@cmp "$(LSAVG_VERIFY_MEDIUM)"    "$(LSAVG_GOLD_MEDIUM)" && echo "Validated $(LSAVG_MEDIUM)"
	@cmp "$(LSMIN_VERIFY_MEDIUM)"    "$(LSMIN_GOLD_MEDIUM)" && echo "Validated $(LSMIN_MEDIUM)"
	@cmp "$(LSMAX_VERIFY_MEDIUM)"    "$(LSMAX_GOLD_MEDIUM)" && echo "Validated $(LSMAX_MEDIUM)"

verify-large: certify-large verify-medium
	@cmp "$(SGB_VERIFY_LARGE)"    "$(SGB_GOLD_LARGE)" && echo "Validated $(SGB_LARGE)"
	@test -n "$(SKIP_SUMMARY_ML)" || (cmp "$(SUMMARY_VERIFY_LARGE)" "$(SUMMARY_GOLD_LARGE)" && echo "Validated $(SUMMARY_LARGE)")
	@cmp "$(JOIN_VERIFY_LARGE)"   "$(JOIN_GOLD_LARGE)" && echo "Validated $(JOIN_LARGE)"
	@cmp "$(CPSLB_VERIFY_LARGE)"   "$(CPSLB_GOLD_LARGE)" && echo "Validated $(CPSLB_LARGE)"
	@cmp "$(LAVG_VERIFY_LARGE)"    "$(LAVG_GOLD_LARGE)" && echo "Validated $(LAVG_LARGE)"
	@cmp "$(LMIN_VERIFY_LARGE)"    "$(LMIN_GOLD_LARGE)" && echo "Validated $(LMIN_LARGE)"
	@cmp "$(LMAX_VERIFY_LARGE)"    "$(LMAX_GOLD_LARGE)" && echo "Validated $(LMAX_LARGE)"
	@cmp "$(LSAVG_VERIFY_LARGE)"    "$(LSAVG_GOLD_LARGE)" && echo "Validated $(LSAVG_LARGE)"
	@cmp "$(LSMIN_VERIFY_LARGE)"    "$(LSMIN_GOLD_LARGE)" && echo "Validated $(LSMIN_LARGE)"
	@cmp "$(LSMAX_VERIFY_LARGE)"    "$(LSMAX_GOLD_LARGE)" && echo "Validated $(LSMAX_LARGE)"

validate: verify

validate-medium: verify-medium

validate-large: verify-large

# ==========================================================================

# Only skip SUMMARY generation for MEDIUM+LARGE (keep SGB generation)
generate-skip-summary-ml: ; @$(MAKE) SKIP_SUMMARY_ML=1 generate-large
certify-skip-summary-ml:  ; @$(MAKE) SKIP_SUMMARY_ML=1 certify-large
verify-skip-summary-ml:   ; @$(MAKE) SKIP_SUMMARY_ML=1 verify-large
validate-skip-summary-ml: ; @$(MAKE) SKIP_SUMMARY_ML=1 validate-large

# ---------- Housekeeping ----------
.PHONY: all generate generate-medium generate-large clean clobber 
 
clean:
	@test ! -d "$(OUT)" || $(RM) "$(OUT)"/*.verify "$(OUT)"/*.sha256

ALL_OUTPUTS := $(BITMAP) $(RAW) $(GBP) \
               $(OUTPUT_SMALL) $(OUTPUT_MEDIUM) $(OUTPUT_LARGE)

clobber: clean
	$(RM) $(ALL_OUTPUTS)
	$(MAKE) -C src clean

.PHONY: help
help:
	@echo "Default: validate-skip-summary-ml (generate small; use data/ for medium+large summaries)"
	@echo "Common:"
	@echo "  make validate            # full validate (generate all summaries)"
	@echo "  make validate-medium     # validate through medium"
	@echo "  make validate-large      # validate through large"
	@echo "  make validate-skip-summary-ml  # generate SGB all sizes, summary small only"
	@echo "  make clobber             # remove output artifacts; keep data/"

