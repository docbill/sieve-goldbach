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

SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c
.ONESHELL:
.DELETE_ON_ERROR:

MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

OUT  := output
DATA := data

# Make one of these your default (pick what you want as the "do the thing" target)
.DEFAULT_GOAL := validate-skip-summary-ml
.PHONY: all
all: validate-skip-summary-ml

# --- Pick all alphas, and the one that mirrors into legacy output/ ---
ALPHAS := $(shell awk 'BEGIN{r=exp(log(2)/8);a=exp(log(2)*-10);eps=1e-12;while(a<1-eps){printf "%.12g ",a;a*=r}print "1"}')
ALPHA_DEFAULT ?= 0.5
# Expand to: --alpha 0.5 --alpha 0.6 ...
ALPHA_ARGS := $(foreach a,$(ALPHAS),--alpha $(a))

# ---------- Parameters ----------
LIMIT        := 200000000
GBCOUNT      := 10000
SMALLSTART   := 4
SPRIMSTART   := 6
SPRIMCOUNT   := 1021020
SMALLCOUNT   := 1000000
MEDIUMCOUNT  := 10000000
MPRIMCOUNT   := 9699690
LARGECOUNT   := 100000000
LPRIMCOUNT   := 111546435

# Size axes
SIZES      := SMALL SPRIM MEDIUM MPRIM LARGE LPRIM
SUFFIX_SMALL  := 1M
SUFFIX_SPRIM  := 17PR2
SUFFIX_MEDIUM := 10M
SUFFIX_MPRIM  := 19PR
SUFFIX_LARGE  := 100M
SUFFIX_LPRIM  := 23PR.5
COUNT_SMALL   := $(SMALLCOUNT)
COUNT_SPRIM   := $(SPRIMCOUNT)
COUNT_MEDIUM  := $(MEDIUMCOUNT)
COUNT_MPRIM   := $(MPRIMCOUNT)
COUNT_LARGE   := $(LARGECOUNT)
COUNT_LPRIM   := $(LPRIMCOUNT)

# ---------- Binaries (real paths) ----------
PRIME_BITMAP_BIN := src/primesieve_bitmap/primesieve_bitmap
STOREPRIMES_BIN  := src/storeprimes/storeprimes
FINDGBPAIRS_BIN  := src/findgbpairs/findgbpairs
CPSLB_BIN        := src/cpslowerbound/cpslowerbound
SUMMARY_BIN      := src/pairrangesummary/pairrangesummary
CERTIFYPRIMES    := src/certifyprimes/certifyprimes
CERTIFYGBPAIRS   := src/certifygbpairs/certifygbpairs
VALIDATESUMMARY  := src/validatepairrangesummary/validatepairrangesummary

PROGRAMS := $(PRIME_BITMAP_BIN) $(STOREPRIMES_BIN) $(FINDGBPAIRS_BIN) \
	    $(CPSLB_BIN) $(CERTIFYPRIMES) $(CERTIFYGBPAIRS) \
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
#   SUMMARY_{SIZE}, CPSLB_{SIZE}, LAMBDA_{TYPE}_{SIZE}
#   their generation rules, and corresponding *_VERIFY targets.

# Helper to read “value of variable named NAME_SIZE”
# Usage: $(call GET,SUFFIX,SMALL) -> 1M
GET = $($(1)_$(2))

# ---------------------------------------------------------------------------

define SIZE_TEMPLATE

# File stems for $(1) = SIZE (e.g., SMALL)
SFX_$(1) := $(call GET,SUFFIX,$(1))
CNT_$(1) := $(call GET,COUNT,$(1))

SGB_FILE_$(1)             := pairrange2sgbll-$$(SFX_$(1)).csv
SGB_TPL_FILE_$(1)         := pairrange2sgbll-$$(SFX_$(1))--=ALPHA=-.csv
SGB_DEFAULT_FILE_$(1)     := pairrange2sgbll-$$(SFX_$(1))-$(ALPHA_DEFAULT).csv
SUMMARY_FILE_$(1)         := pairrangesummary-$$(SFX_$(1)).csv
SUMMARY_TPL_FILE_$(1)     := pairrangesummary-$$(SFX_$(1))--=ALPHA=-.csv
SUMMARY_DEFAULT_FILE_$(1) := pairrangesummary-$$(SFX_$(1))-$(ALPHA_DEFAULT).csv
JOIN_FILE_$(1)            := pairrangejoin-$$(SFX_$(1)).csv
CPSLB_FILE_$(1)           := cpslowerbound-$$(SFX_$(1)).csv
LAVG_FILE_$(1)            := lambdaavg-$$(SFX_$(1)).csv
LMIN_FILE_$(1)            := lambdamin-$$(SFX_$(1)).csv
LMAX_FILE_$(1)            := lambdamax-$$(SFX_$(1)).csv
LSAVG_FILE_$(1)           := lambdastatsavg-$$(SFX_$(1)).csv
LSMIN_FILE_$(1)           := lambdastatsmin-$$(SFX_$(1)).csv
LSMAX_FILE_$(1)           := lambdastatsmax-$$(SFX_$(1)).csv

SGB_$(1)             := $(OUT)/$$(SGB_FILE_$(1))
SGB_TPL_$(1)         := $(OUT)/alpha--=ALPHA=-/$$(SGB_TPL_FILE_$(1))
SGB_DEFAULT_$(1)     := $(OUT)/alpha-$(ALPHA_DEFAULT)/$$(SGB_DEFAULT_FILE_$(1))
SUMMARY_$(1)         := $(OUT)/$$(SUMMARY_FILE_$(1))
SUMMARY_TPL_$(1)     := $(OUT)/alpha--=ALPHA=-/$$(SUMMARY_TPL_FILE_$(1))
SUMMARY_DEFAULT_$(1) := $(OUT)/alpha-$(ALPHA_DEFAULT)/$$(SUMMARY_DEFAULT_FILE_$(1))
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
	@(echo -n sha256= | tee -a "$$@")
	sha256sum < "$$(SUMMARY_$(1))" | tee -a "$$@"
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

# Fast summary counts via copy+append using
# - SMALL: generate with header up to SMALLCOUNT
# - MEDIUM: depend on SMALL; copy SMALL header, then append from SMALLCOUNT..MEDIUMCOUNT
# - LARGE:  depend on MEDIUM; copy MEDIUM header, then append from MEDIUMCOUNT..LARGECOUNT
# - If SKIP_SUMMARY_ML=1 and SIZE is MEDIUM/LARGE: DO NOT define a rule (read data/)

# SMALL
$(SUMMARY_SMALL): $(SUMMARY_BIN) $(RAW) | $(OUT) 
	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do mkdir -p "$(OUT)/alpha-$$a"; done
	./$(SUMMARY_BIN) $(ALPHA_ARGS) --compat=v0.1.5 --model=empirical \
	  --dec-n-start $(SMALLSTART) --dec-n-end $(SMALLCOUNT) \
	  --prim-n-start $(SPRIMSTART) --prim-n-end $(SPRIMCOUNT) \
	  --trace=primorial \
	  --dec-out="$(SUMMARY_TPL_SMALL)" --prim-out="$(SUMMARY_TPL_SPRIM)" "$(RAW)"
	@cp "$(SUMMARY_DEFAULT_SMALL)" "$(SUMMARY_SMALL)"

# Emit the MEDIUM rule only when we actually build it (not when using data/)
ifneq ($(SKIP_SUMMARY_ML),1)
# make sure SUMMARY_MEDIUM resolves under output/, not data/
ifeq ($(patsubst $(OUT)/%,ok,$(SUMMARY_MEDIUM)),ok)

$(SUMMARY_MEDIUM): $(SUMMARY_BIN) $(RAW) $(SUMMARY_SMALL) | $(OUT)
	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
	  src="$(SUMMARY_TPL_SMALL)"; dst="$(SUMMARY_TPL_MEDIUM)"; \
	  src="$${src//-=ALPHA=-/$$a}"; dst="$${dst//-=ALPHA=-/$$a}"; \
	  ( dir="$$dst"; dir="$${dir%/*}"; [ -d "$$dir" ] || mkdir -p "$$dir" ) ; \
	  cp "$$src" "$$dst"; \
	  src="$(SUMMARY_TPL_SPRIM)"; dst="$(SUMMARY_TPL_MPRIM)"; \
	  src="$${src//-=ALPHA=-/$$a}"; dst="$${dst//-=ALPHA=-/$$a}"; \
	  ( dir="$$dst"; dir="$${dir%/*}"; [ -d "$$dir" ] || mkdir -p "$$dir" ) ; \
	  cp "$$src" "$$dst"; \
	done
	@cp "$(SUMMARY_SMALL)" "$(SUMMARY_MEDIUM)"
	@head -n 1 < "$(SUMMARY_MEDIUM)"
	./$(SUMMARY_BIN) $(ALPHA_ARGS) --compat=v0.1.5 --model=empirical \
	  --dec-n-start $(SMALLCOUNT) --dec-n-end $(MEDIUMCOUNT) \
	  --prim-n-start $(SPRIMCOUNT) --prim-n-end $(MPRIMCOUNT) \
	  --append --trace=primorial \
	  --dec-out="$(SUMMARY_TPL_MEDIUM)" --prim-out="$(SUMMARY_TPL_MPRIM)" "$(RAW)"
	@cp "$(SUMMARY_DEFAULT_MEDIUM)" "$(SUMMARY_MEDIUM)"

endif
endif

# Emit the LARGE rule only when we actually build it (not when using data/)
ifneq ($(SKIP_SUMMARY_ML),1)
# make sure SUMMARY_LARGE resolves under output/, not data/
ifeq ($(patsubst $(OUT)/%,ok,$(SUMMARY_LARGE)),ok)

$(SUMMARY_LARGE): $(SUMMARY_BIN) $(RAW) $(SUMMARY_MEDIUM) | $(OUT)
	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
	  src="$(SUMMARY_TPL_MEDIUM)"; dst="$(SUMMARY_TPL_LARGE)"; \
	  src="$${src//-=ALPHA=-/$$a}"; dst="$${dst//-=ALPHA=-/$$a}"; \
	  ( dir="$$dst"; dir="$${dir%/*}"; [ -d "$$dir" ] || mkdir -p "$$dir" ) ; \
	  cp "$$src" "$$dst"; \
	  src="$(SUMMARY_TPL_MPRIM)"; dst="$(SUMMARY_TPL_LPRIM)"; \
	  src="$${src//-=ALPHA=-/$$a}"; dst="$${dst//-=ALPHA=-/$$a}"; \
	  ( dir="$$dst"; dir="$${dir%/*}"; [ -d "$$dir" ] || mkdir -p "$$dir" ) ; \
	  cp "$$src" "$$dst"; \
	done
	@cp "$(SUMMARY_MEDIUM)" "$(SUMMARY_LARGE)"
	@head -n 1 < "$(SUMMARY_LARGE)"
	./$(SUMMARY_BIN) $(ALPHA_ARGS) --compat=v0.1.5 --model=empirical \
	  --dec-n-start $(MEDIUMCOUNT) --dec-n-end $(LARGECOUNT) \
	  --prim-n-start $(MPRIMCOUNT) --prim-n-end $(LPRIMCOUNT) \
	  --append --trace=primorial \
	  --dec-out="$(SUMMARY_TPL_LARGE)" --prim-out="$(SUMMARY_TPL_LPRIM)" "$(RAW)"
	@cp "$(SUMMARY_DEFAULT_LARGE)" "$(SUMMARY_LARGE)"

endif
endif

# Predicted HL-A pairs
$(SGB_SMALL): $(SUMMARY_BIN) $(RAW) | $(OUT)
	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do mkdir -p "$(OUT)/alpha-$$a"; done
	./$(SUMMARY_BIN) $(ALPHA_ARGS) --compat=v0.1.5 --model=hl-a \
	--dec-n-start $(SMALLSTART) --dec-n-end $(SMALLCOUNT) \
	--prim-n-start $(SPRIMSTART) --prim-n-end $(SPRIMCOUNT) \
	--trace=primorial \
	--dec-out="$(SGB_TPL_SMALL)" --prim-out="$(SGB_TPL_SPRIM)" "$(RAW)" 
	@cp "$(SGB_DEFAULT_SMALL)" "$(SGB_SMALL)"

$(SGB_MEDIUM): $(SUMMARY_BIN) $(RAW) $(SGB_SMALL) | $(OUT)
	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
	  src="$(SGB_TPL_SMALL)"; dst="$(SGB_TPL_MEDIUM)"; \
	  src="$${src//-=ALPHA=-/$$a}"; dst="$${dst//-=ALPHA=-/$$a}"; \
	  ( dir="$$dst"; dir="$${dir%/*}"; [ -d "$$dir" ] || mkdir -p "$$dir" ) ; \
	  cp "$$src" "$$dst"; \
	  src="$(SGB_TPL_SPRIM)"; dst="$(SGB_TPL_MPRIM)"; \
	  src="$${src//-=ALPHA=-/$$a}"; dst="$${dst//-=ALPHA=-/$$a}"; \
	  ( dir="$$dst"; dir="$${dir%/*}"; [ -d "$$dir" ] || mkdir -p "$$dir" ) ; \
	  cp "$$src" "$$dst"; \
	done
	@cp "$(SGB_SMALL)" "$(SGB_MEDIUM)"
	@head -1 < "$(SGB_MEDIUM)"
	./$(SUMMARY_BIN) $(ALPHA_ARGS) --compat=v0.1.5 --model=hl-a \
	  --dec-n-start $(SMALLCOUNT) --dec-n-end $(MEDIUMCOUNT) \
	  --prim-n-start $(SPRIMCOUNT) --prim-n-end $(MPRIMCOUNT) \
	  --append --trace=primorial \
	  --dec-out="$(SGB_TPL_MEDIUM)" --prim-out="$(SGB_TPL_MPRIM)" "$(RAW)"
	@cp "$(SGB_DEFAULT_MEDIUM)" "$(SGB_MEDIUM)"

$(SGB_LARGE): $(SUMMARY_BIN) $(RAW) $(SGB_MEDIUM)
	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
	  src="$(SGB_TPL_MEDIUM)"; dst="$(SGB_TPL_LARGE)"; \
	  src="$${src//-=ALPHA=-/$$a}"; dst="$${dst//-=ALPHA=-/$$a}"; \
	  ( dir="$$dst"; dir="$${dir%/*}"; [ -d "$$dir" ] || mkdir -p "$$dir" ) ; \
	  cp "$$src" "$$dst"; \
	  src="$(SGB_TPL_MPRIM)"; dst="$(SGB_TPL_LPRIM)"; \
	  src="$${src//-=ALPHA=-/$$a}"; dst="$${dst//-=ALPHA=-/$$a}"; \
	  ( dir="$$dst"; dir="$${dir%/*}"; [ -d "$$dir" ] || mkdir -p "$$dir" ) ; \
	  cp "$$src" "$$dst"; \
	done
	@cp "$(SGB_MEDIUM)" "$(SGB_LARGE)"
	@head -1 < "$(SGB_LARGE)"
	./$(SUMMARY_BIN) $(ALPHA_ARGS) --compat=v0.1.5 --model=hl-a \
	  --dec-n-start $(MEDIUMCOUNT) --dec-n-end $(LARGECOUNT) \
	  --prim-n-start $(MPRIMCOUNT) --prim-n-end $(LPRIMCOUNT) \
	  --append --trace=primorial \
	  --dec-out="$(SGB_TPL_LARGE)" --prim-out="$(SGB_TPL_LPRIM)" "$(RAW)"
	@cp "$(SGB_DEFAULT_LARGE)" "$(SGB_LARGE)"


# ---------- Generic sha256 rule ----------
%.sha256: %
	sha256sum "$<" | tee "$@"

# ---------- Top-level generation groups ----------
generate:       $(BITMAP) $(RAW) $(GBP) $(SGB_SMALL) $(SGB_SPRIM) $(SUMMARY_SMALL) \
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
	@(echo -n "sha256=" | tee -a "$@")
	sha256sum < "$(BITMAP)"| tee -a "$@"

$(RAW_VERIFY): $(CERTIFYPRIMES) $(RAW) | $(OUT)
	./$(CERTIFYPRIMES) --binary --file "$(RAW)" | tee "$@"
	@(echo -n "sha256=" | tee -a "$@")
	sha256sum < "$(RAW)"| tee -a "$@"

$(GBP_VERIFY): $(CERTIFYGBPAIRS) $(GBP) $(BITMAP) | $(OUT)
	./$(CERTIFYGBPAIRS) --bitmap "$(BITMAP)" --file "$(GBP)" | tee "$@"
	@(echo -n "sha256=" | tee -a "$@")
	sha256sum < "$(BITMAP)"| tee -a "$@"

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


# ---- Config ----------------------------------------------------
PAPER_PROJECT := sieve-goldbach
DATE    := $(shell date +%Y%m%d)
PAPER_SRC_DIR := $(PAPER_PROJECT)-src
PAPER_ZIP     := $(PAPER_PROJECT)-src-$(DATE).zip
PAPER_SHA256     := $(PAPER_PROJECT)-src-$(DATE).sha256

# List every file your TeX build needs (add .sty/.cls/.bst if any)
PAPER_FILES := \
  LICENSES/CC-BY-4.0.txt \
  paper/sieve_goldbach.tex \
  paper/sieve_goldbach.bib \
  paper/README.txt \
  paper/Makefile \
  paper/cpslowerbound-100M.csv \
  paper/lambdaavg-100M.csv \
  paper/lambdamax-100M.csv \
  paper/lambdamin-100M.csv \
  paper/pairrangejoin-100M.csv

.PHONY: zip-src clean-src

# ---- Build the date-stamped source archive ---------------------
zip-src:
	@rm -rf "$(OUT)/$(PAPER_SRC_DIR)"
	@mkdir -p "$(OUT)/$(PAPER_SRC_DIR)"
	@cp -f $(PAPER_FILES) "$(OUT)/$(PAPER_SRC_DIR)/."
	@cd "$(OUT)/$(PAPER_SRC_DIR)" && ( \
	  echo "SHA256 manifest for $(PAPER_PROJECT) ($(DATE))" > CHECKSUMS.txt; \
	  for f in $(notdir $(PAPER_FILES)); do sha256 "$$f" >> CHECKSUMS.txt; done )
	@(cd "$(OUT)" && zip -r -X "$(PAPER_ZIP)" "$(PAPER_SRC_DIR)" >/dev/null)
	@rm -rf "$(OUT)/$(PAPER_SRC_DIR)"
	@echo Created "$(OUT)/$(PAPER_ZIP)"
	@(cd "$(OUT)" && shasum -a 256 "$(PAPER_ZIP)"|tee "$(PAPER_SHA256)")

# ---- Cleanup temp folder ---------------------------------------
clean-src:
	@rm -rf "$(OUT)/$(PAPER_ZIP)" "$(OUT)/$(PAPER_SHA256)"

