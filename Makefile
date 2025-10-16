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
.DEFAULT_GOAL := validate
.PHONY: all
all: validate

# --- Pick all alphas, and the one that mirrors into legacy output/ ---
# ALPHAS := $(shell awk 'BEGIN{r=exp(log(2)/8);a=exp(log(2)*-10);eps=1e-12;while(a<0.015625-eps){printf "%.12g ",a;a*=r}print "1"}')
ALPHAS := $(shell awk 'BEGIN{r=exp(log(2)/8);a=exp(log(2)*-10);eps=1e-12;while(a<1-eps){printf "%.12g ",a;a*=r}print "1"}')
ALPHA_DEFAULT ?= 0.5
# Expand to: --alpha 0.5 --alpha 0.6 ...
ALPHA_ARGS := $(foreach a,$(ALPHAS),--alpha $(a))

# ---------- Parameters ----------
LIMIT        := 250000000
GBCOUNT      := 10000
FORMATS_HL_A := full raw norm
FORMATS_EMP  := full raw norm cps
COMPAT       := v0.1.5


START_SMALL   := 4

SUFFIX_SMALLA := 500K
START_SMALLA := $(START_SMALL)
END_SMALLA := 500000

SUFFIX_SMALLB := 700K
START_SMALLB := $(END_SMALLA)
END_SMALLB := 700000

SUFFIX_SMALLC := 800K
START_SMALLC := $(END_SMALLB)
END_SMALLC := 800000

SUFFIX_SMALLD := 900K
START_SMALLD := $(END_SMALLC)
END_SMALLD := 900000

SUFFIX_SMALLE := 1000K
START_SMALLE := $(END_SMALLD)
END_SMALLE := 1000000

SUFFIX_SMALL  := 1M
END_SMALL     := $(END_SMALLE)


START_SPRIM   := 4

SUFFIX_SPRIMA := 13PR34D2
START_SPRIMA := $(START_SPRIM)
END_SPRIMA := 510510

SUFFIX_SPRIMB := 13PR47D2
START_SPRIMB := $(END_SPRIMA)
END_SPRIMB := 705705

SUFFIX_SPRIMC := 13PR53D2
START_SPRIMC := $(END_SPRIMB)
END_SPRIMC := 795795

SUFFIX_SPRIMD := 13PR60D2
START_SPRIMD := $(END_SPRIMC)
END_SPRIMD := 900900

SUFFIX_SPRIME := 13PR68D2
START_SPRIME := $(END_SPRIMD)
END_SPRIME := 1021020

SUFFIX_SPRIM  := 17PR2
END_SPRIM     := $(END_SPRIME)

SMALLPARTS := A B C D E
SPRIMPARTS := A B C D E


SUFFIX_MEDIUMA := 5M
START_MEDIUMA := $(END_SMALL)
END_MEDIUMA := 5000000

SUFFIX_MEDIUMB := 7M
START_MEDIUMB := $(END_MEDIUMA)
END_MEDIUMB := 7000000

SUFFIX_MEDIUMC := 8M
START_MEDIUMC := $(END_MEDIUMB)
END_MEDIUMC := 8000000

SUFFIX_MEDIUMD := 9M
START_MEDIUMD := $(END_MEDIUMC)
END_MEDIUMD := 9000000

SUFFIX_MEDIUME := 10M
START_MEDIUME := $(END_MEDIUMD)
END_MEDIUME := 10000000

SUFFIX_MPRIMA := 17PR20D2
START_MPRIMA := $(END_SPRIM)
END_MPRIMA := 5105100

SUFFIX_MPRIMB := 17PR27D2
START_MPRIMB := $(END_MPRIMA)
END_MPRIMB := 6891885

SUFFIX_MPRIMC := 17PR31D2
START_MPRIMC := $(END_MPRIMB)
END_MPRIMC := 7912905

SUFFIX_MPRIMD := 17PR35D2
START_MPRIMD := $(END_MPRIMC)
END_MPRIMD := 8933925

SUFFIX_MPRIME := 17PR38D2
START_MPRIME := $(END_MPRIMD)
END_MPRIME := 9699690

SUFFIX_MEDIUM := 10M
END_MEDIUM  := $(END_MEDIUME)

SUFFIX_MPRIM  := 19PR
END_MPRIM   := $(END_MPRIME)

MEDIUMPARTS := A B C D E
MPRIMPARTS := A B C D E


SUFFIX_LARGEA := 50M
START_LARGEA := $(END_MEDIUM)
END_LARGEA := 50000000

SUFFIX_LARGEB := 70M
START_LARGEB := $(END_LARGEA)
END_LARGEB := 70000000

SUFFIX_LARGEC := 80M
START_LARGEC := $(END_LARGEB)
END_LARGEC := 80000000

SUFFIX_LARGED := 90M
START_LARGED := $(END_LARGEC)
END_LARGED := 90000000

SUFFIX_LARGEE := 100M
START_LARGEE := $(END_LARGED)
END_LARGEE := 100000000

SUFFIX_LARGEF := nonsensical
START_LARGEF := $(END_LARGEE)
END_LARGEF := 110000000

SUFFIX_LPRIMA := 17PR196D2
START_LPRIMA := $(END_MPRIM)
END_LPRIMA := 50029980

SUFFIX_LPRIMB := 17PR274D2
START_LPRIMB := $(END_LPRIMA)
END_LPRIMB := 69939870

SUFFIX_LPRIMC := 17PR314D2
START_LPRIMC := $(END_LPRIMB)
END_LPRIMC := 80150070

SUFFIX_LPRIMD := 17PR352D2
START_LPRIMD := $(END_LPRIMC)
END_LPRIMD := 89849760

SUFFIX_LPRIME := 17PR400D2
START_LPRIME := $(END_LPRIMD)
END_LPRIME := 102102000

SUFFIX_LPRIMF := 17PR437D2
START_LPRIMF := $(END_LPRIME)
END_LPRIMF := 111546435

SUFFIX_LARGE  := 100M
END_LARGE   := $(END_LARGEF)

SUFFIX_LPRIM  := 23PR.5
END_LPRIM   := $(END_LPRIMF)

LARGEPARTS := A B C D E F
LPRIMPARTS := A B C D E F

# Size axes
SIZES         := SMALL SPRIM MEDIUM MPRIM LARGE LPRIM

# ---------- Binaries (real paths) ----------
PRIME_BITMAP_BIN := src/primesieve_bitmap/primesieve_bitmap
STOREPRIMES_BIN  := src/storeprimes/storeprimes
FINDGBPAIRS_BIN  := src/findgbpairs/findgbpairs
CPSLB_BIN        := src/cpslowerbound/cpslowerbound
SUMMARY_BIN      := src/gbpairsummary/gbpairsummary
CERTIFYPRIMES    := src/certifyprimes/certifyprimes
CERTIFYGBPAIRS   := src/certifygbpairs/certifygbpairs
VALIDATESUMMARY  := src/validatepairrangesummary/validatepairrangesummary
MERGECPS         := src/mergecps/mergecps

PROGRAMS := $(PRIME_BITMAP_BIN) $(STOREPRIMES_BIN) $(FINDGBPAIRS_BIN) \
	    $(CPSLB_BIN) $(CERTIFYPRIMES) $(CERTIFYGBPAIRS) \
	    $(SUMMARY_BIN) $(VALIDATESUMMARY) $(MERGECPS)

# Dispatcher: build any src/<prog>/<prog> via src/Makefile (inherits toolchain)
$(PROGRAMS):
	$(MAKE) -C src $(notdir $@)

# ---------- Backup ----------
BACKUP_DIR := backups
BACKUP_TPL := backup-*-$(COMPAT).tar.bz2
BACKUP_FILE := $(BACKUP_DIR)/backup-$(shell date -u +%Y%m%d%H%M%S)-$(COMPAT).tar.bz2

LAST_BACKUP_FILE := $(shell \
	if [ -d "$(BACKUP_DIR)" ] ; then \
		find "$(BACKUP_DIR)" -name "$(BACKUP_TPL)" -type f 2>/dev/null | \
			sort | tail -n1 ; \
	fi )

$(BACKUP_FILE):
	@mkdir -p "$(BACKUP_DIR)"
	@find "$(OUT)" -name \*.partial.csv -print0|xargs -0 tar cvvfj "$(BACKUP_FILE)"
	@echo "Created \"$(BACKUP_FILE)\""
	test -z "$(LAST_BACKUP_FILE)" || if ( cmp "$(LAST_BACKUP_FILE)" "$(BACKUP_FILE)" ); then $(RM) "$(LAST_BACKUP_FILE)" ; fi

backup: $(BACKUP_FILE)

restore: 
	@test -n "$(LAST_BACKUP_FILE)" || (echo "$(BACKUP_TPL) not found." 1>&2 && exit 1)
	@tar xvvfj "$(LAST_BACKUP_FILE)"

# ---------- Base outputs (non-S/M/L) ----------
BITMAP_FILE := primes-250M.bitmap
RAW_FILE    := primes-250M.raw
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

# Helper to read "value of variable named NAME_SIZE"
# Usage: $(call GET,SUFFIX,SMALL) -> 1M
GET = $($(1)_$(2))
SFX = $(SUFFIX_$(1))

# Helper to conditionally include dependencies when SIZE parameter is not blank
# Tests the SIZE parameter itself, not the constructed variable
# Usage in template: $(call IFSIZE,$(2),$$(VAR_$(2)A).partial.csv)
# If $(2) is empty: returns empty string
# If $(2) is non-empty: returns the second parameter as-is
IFSIZE = $(if $(strip $(1)),$(2))
# ---------------------------------------------------------------------------

LOWER_CASE = $(shell echo $(1) | tr '[:upper:]' '[:lower:]')


define SUMMARY_TEMPLATE
# File stems for $(1) = SIZE (e.g., SMALL)
SFX_$(1) := $$(call GET,SUFFIX,$(1))

SUMMARY_TPL_FILE_$(1)     := gbpairsummary-$$(SFX_$(1))-empirical--=FORMAT=---=ALPHA=--$(COMPAT)
SUMMARY_DEFAULT_FILE_$(1) := gbpairsummary-$$(SFX_$(1))-empirical-full-$(ALPHA_DEFAULT)-$(COMPAT)
SUMMARY_TPL_$(1)          := $(OUT)/alpha--=ALPHA=-/$$(SUMMARY_TPL_FILE_$(1))
SUMMARY_DEFAULT_$(1)      := $(OUT)/alpha-$(ALPHA_DEFAULT)/$$(SUMMARY_DEFAULT_FILE_$(1))
CPS_SUMMARY_FILE_$(1)     := cpssummary-$$(SFX_$(1))-$(COMPAT)
CPS_SUMMARY_$(1)          := $(OUT)/$$(CPS_SUMMARY_FILE_$(1))
SGB_TPL_FILE_$(1)         := gbpairsummary-$$(SFX_$(1))-hl-a--=FORMAT=---=ALPHA=--$(COMPAT)
SGB_DEFAULT_FILE_$(1)     := gbpairsummary-$$(SFX_$(1))-hl-a-full-$(ALPHA_DEFAULT)-$(COMPAT)
SGB_TPL_$(1)              := $(OUT)/alpha--=ALPHA=-/$$(SGB_TPL_FILE_$(1))
SGB_DEFAULT_$(1)          := $(OUT)/alpha-$(ALPHA_DEFAULT)/$$(SGB_DEFAULT_FILE_$(1))
CPSLB_TPL_FILE_$(1)       := gbpairsummary-$$(SFX_$(1))-empirical-cps--=ALPHA=--$(COMPAT)
CPSLB_DEFAULT_FILE_$(1)   := gbpairsummary-$$(SFX_$(1))-empirical-cps-$(ALPHA_DEFAULT)-$(COMPAT)
CPSLB_TPL_$(1)            := $(OUT)/alpha--=ALPHA=-/$$(CPSLB_TPL_FILE_$(1))
CPSLB_DEFAULT_$(1)        := $(OUT)/alpha-$(ALPHA_DEFAULT)/$$(CPSLB_DEFAULT_FILE_$(1))
JOIN_TPL_FILE_$(1)        := pairrangejoin-$$(SFX_$(1))--=ALPHA=--$(COMPAT)
JOIN_DEFAULT_FILE_$(1)    := pairrangejoin-$$(SFX_$(1))-$(ALPHA_DEFAULT)-$(COMPAT)
JOIN_TPL_$(1)             := $(OUT)/alpha--=ALPHA=-/$$(JOIN_TPL_FILE_$(1))
JOIN_DEFAULT_$(1)         := $(OUT)/alpha-$(ALPHA_DEFAULT)/$$(JOIN_DEFAULT_FILE_$(1))
endef

define SIZE_TEMPLATE2

SUMMARY_FILE_$(1)         := pairrangesummary-$$(SFX_$(1))-$(COMPAT)
SUMMARY_$(1)              := $(OUT)/$$(SUMMARY_FILE_$(1))
SGB_FILE_$(1)             := pairrange2sgbll-$$(SFX_$(1))-$(COMPAT)
SGB_$(1)                  := $(OUT)/$$(SGB_FILE_$(1))
JOIN_FILE_$(1)            := pairrangejoin-$$(SFX_$(1))-$(COMPAT)
JOIN_$(1)                 := $(OUT)/$$(JOIN_FILE_$(1))
CPSLB_FILE_$(1)           := cpslowerbound-$$(SFX_$(1))-$(COMPAT)
CPSLB_$(1)                := $(OUT)/$$(CPSLB_FILE_$(1))


# Alpha-specific lambda files
LAVG_FILE_$(1)            := lambdaavg-$$(SFX_$(1))-$(COMPAT)
LMIN_FILE_$(1)            := lambdamin-$$(SFX_$(1))-$(COMPAT)
LMAX_FILE_$(1)            := lambdamax-$$(SFX_$(1))-$(COMPAT)
LSAVG_FILE_$(1)           := lambdastatsavg-$$(SFX_$(1))-$(COMPAT)
LSMIN_FILE_$(1)           := lambdastatsmin-$$(SFX_$(1))-$(COMPAT)
LSMAX_FILE_$(1)           := lambdastatsmax-$$(SFX_$(1))-$(COMPAT)

LAVG_TPL_FILE_$(1)       := lambdaavg-$$(SFX_$(1))--=ALPHA=--$(COMPAT)
LMIN_TPL_FILE_$(1)       := lambdamin-$$(SFX_$(1))--=ALPHA=--$(COMPAT)
LMAX_TPL_FILE_$(1)       := lambdamax-$$(SFX_$(1))--=ALPHA=--$(COMPAT)
LSAVG_TPL_FILE_$(1)      := lambdastatsavg-$$(SFX_$(1))--=ALPHA=--$(COMPAT)
LSMIN_TPL_FILE_$(1)      := lambdastatsmin-$$(SFX_$(1))--=ALPHA=--$(COMPAT)
LSMAX_TPL_FILE_$(1)      := lambdastatsmax-$$(SFX_$(1))--=ALPHA=--$(COMPAT)

LAVG_DEFAULT_FILE_$(1)   := lambdaavg-$$(SFX_$(1))-$(ALPHA_DEFAULT)-$(COMPAT)
LMIN_DEFAULT_FILE_$(1)   := lambdamin-$$(SFX_$(1))-$(ALPHA_DEFAULT)-$(COMPAT)
LMAX_DEFAULT_FILE_$(1)   := lambdamax-$$(SFX_$(1))-$(ALPHA_DEFAULT)-$(COMPAT)
LSAVG_DEFAULT_FILE_$(1)  := lambdastatsavg-$$(SFX_$(1))-$(ALPHA_DEFAULT)-$(COMPAT)
LSMIN_DEFAULT_FILE_$(1)  := lambdastatsmin-$$(SFX_$(1))-$(ALPHA_DEFAULT)-$(COMPAT)
LSMAX_DEFAULT_FILE_$(1)  := lambdastatsmax-$$(SFX_$(1))-$(ALPHA_DEFAULT)-$(COMPAT)

LAVG_$(1)     := $(OUT)/$$(LAVG_FILE_$(1))
LMIN_$(1)     := $(OUT)/$$(LMIN_FILE_$(1))
LMAX_$(1)     := $(OUT)/$$(LMAX_FILE_$(1))
LSAVG_$(1)     := $(OUT)/$$(LSAVG_FILE_$(1))
LSMIN_$(1)     := $(OUT)/$$(LSMIN_FILE_$(1))
LSMAX_$(1)     := $(OUT)/$$(LSMAX_FILE_$(1))

# Alpha-0.5 lambda file paths for copying to top-level
LAVG_TPL_$(1)        := $(OUT)/alpha--=ALPHA=-/$$(LAVG_TPL_FILE_$(1))
LAVG_DEFAULT_$(1)    := $(OUT)/alpha-$(ALPHA_DEFAULT)/$$(LAVG_DEFAULT_FILE_$(1))
LMIN_TPL_$(1)        := $(OUT)/alpha--=ALPHA=-/$$(LMIN_TPL_FILE_$(1))
LMIN_DEFAULT_$(1)        := $(OUT)/alpha-$(ALPHA_DEFAULT)/$$(LMIN_DEFAULT_FILE_$(1))
LMAX_TPL_$(1)        := $(OUT)/alpha--=ALPHA=-/$$(LMAX_TPL_FILE_$(1))
LMAX_DEFAULT_$(1)        := $(OUT)/alpha-$(ALPHA_DEFAULT)/$$(LMAX_DEFAULT_FILE_$(1))
LSAVG_TPL_$(1)       := $(OUT)/alpha--=ALPHA=-/$$(LSAVG_TPL_FILE_$(1))
LSAVG_DEFAULT_$(1)       := $(OUT)/alpha-$(ALPHA_DEFAULT)/$$(LSAVG_DEFAULT_FILE_$(1))
LSMIN_TPL_$(1)       := $(OUT)/alpha--=ALPHA=-/$$(LSMIN_TPL_FILE_$(1))
LSMIN_DEFAULT_$(1)       := $(OUT)/alpha-$(ALPHA_DEFAULT)/$$(LSMIN_DEFAULT_FILE_$(1))
LSMAX_TPL_$(1)       := $(OUT)/alpha--=ALPHA=-/$$(LSMAX_TPL_FILE_$(1))
LSMAX_DEFAULT_$(1)       := $(OUT)/alpha-$(ALPHA_DEFAULT)/$$(LSMAX_DEFAULT_FILE_$(1))

OUTPUT_$(1)   := $$(SGB_$(1)).csv $$(SGB_DEFAULT_$(1)).csv $$(SUMMARY_$(1)).csv $$(SUMMARY_DEFAULT_$(1)).csv \
	$$(JOIN_$(1)).csv $$(CPSLB_$(1)).csv \
	$$(LAVG_$(1)).csv $$(LMIN_$(1)).csv $$(LMAX_$(1)).csv \
	$$(LSAVG_$(1)).csv $$(LSMIN_$(1)).csv $$(LSMAX_$(1)).csv

# Verifies (sha256 or tool-specific)
JOIN_VERIFY_$(1)     := $$(JOIN_$(1)).csv.sha256
CPSLB_VERIFY_$(1)    := $$(CPSLB_$(1)).csv.sha256
CPS_SUMMARY_VERIFY_$(1)  := $$(CPS_SUMMARY_$(1)).csv.sha256
LAVG_VERIFY_$(1)     := $$(LAVG_$(1)).csv.sha256
LMIN_VERIFY_$(1)     := $$(LMIN_$(1)).csv.sha256
LMAX_VERIFY_$(1)     := $$(LMAX_$(1)).csv.sha256
LSAVG_VERIFY_$(1)     := $$(LSAVG_$(1)).csv.sha256
LSMIN_VERIFY_$(1)     := $$(LSMIN_$(1)).csv.sha256
LSMAX_VERIFY_$(1)     := $$(LSMAX_$(1)).csv.sha256

# Corresponding Gold references
SGB_GOLD_$(1)     := $(DATA)/$$(SGB_FILE_$(1)).csv.verify
SUMMARY_GOLD_$(1) := $(DATA)/$$(SUMMARY_FILE_$(1)).csv.verify
JOIN_GOLD_$(1)    := $(DATA)/$$(JOIN_FILE_$(1)).csv.sha256
CPSLB_GOLD_$(1)   := $(DATA)/$$(CPSLB_FILE_$(1)).csv.sha256
LAVG_GOLD_$(1)    := $(DATA)/$$(LAVG_FILE_$(1)).csv.sha256
LMIN_GOLD_$(1)    := $(DATA)/$$(LMIN_FILE_$(1)).csv.sha256
LMAX_GOLD_$(1)    := $(DATA)/$$(LMAX_FILE_$(1)).csv.sha256
LSAVG_GOLD_$(1)    := $(DATA)/$$(LSAVG_FILE_$(1)).csv.sha256
LSMIN_GOLD_$(1)    := $(DATA)/$$(LSMIN_FILE_$(1)).csv.sha256
LSMAX_GOLD_$(1)    := $(DATA)/$$(LSMAX_FILE_$(1)).csv.sha256

endef

# Expand the per-size template for all sizes
$(foreach SZ,$(SIZES),$(eval $(call SUMMARY_TEMPLATE,$(SZ))))

# Expand the per-size template for all sizes
$(foreach SZ,$(SIZES),$(eval $(call SIZE_TEMPLATE2,$(SZ))))

# --- Rebind only after all per-size vars are defined ---


define SIZE_TEMPLATE3

# --- Generation rules ---
$$(SUMMARY_$(1)).csv: $$(SUMMARY_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

$$(SGB_$(1)).csv: $$(SGB_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

# Join summary as sgb files (skip MEDIUM/LARGE when SKIP_SUMMARY_ML is not set)
# $$(JOIN_$(1)): $$(SUMMARY_$(1)).csv $$(SGB_$(1)).csv $(SUMMARY_DEFAULT_$(1)).csv $(SGB_DEFAULT_$(1)).csv | $(OUT)
# 	chmod ugo+x ./bin/joinSumPred.awk
# 	./bin/joinSumPred.awk "$$(call GET,SUMMARY,$(1)).csv" "$$(call GET,SGB,$(1)).csv" > "$$@"

$$(JOIN_DEFAULT_$(1)).csv: $$(SUMMARY_DEFAULT_$(1)).csv $(SGB_DEFAULT_$(1)).csv bin/joinSumPred.awk
	@chmod ugo+x ./bin/joinSumPred.awk
	@set -Eeuo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		summary_src="$$(call GET,SUMMARY_TPL,$(1))"; sgb_src="$$(call GET,SGB_TPL,$(1))"; \
		summary_src="$$$${summary_src//-=ALPHA=-/$$$$a}"; sgb_src="$$$${sgb_src//-=ALPHA=-/$$$$a}"; \
		summary_src="$$$${summary_src//-=FORMAT=-/full}"; sgb_src="$$$${sgb_src//-=FORMAT=-/full}"; \
		[ -r "$$$$summary_src.csv" ] || (echo "Failed to find $$$$summary_src.csv" >&2; exit 1) ; \
		[ -r "$$$$sgb_src.csv" ] || (echo "Failed to find $$$$sgb_src.csv" >&2; exit 1) ; \
		dst="$$(call GET,JOIN_TPL,$(1))"; \
		dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
		./bin/joinSumPred.awk -v alpha=$$$$a "$$$$summary_src.csv" "$$$$sgb_src.csv" > "$$$$dst.csv"; \
	done

$$(JOIN_$(1)).csv: $$(JOIN_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

# CPS lower bound (needs n_0 from SUMMARY)
$$(CPSLB_$(1)).csv: $(CPSLB_BIN) $(RAW) $$(SUMMARY_$(1)).csv $(SUMMARY_DEFAULT_$(1)).csv
	cp "$$(call GET,CPSLB_DEFAULT,$(1)).csv" "$$@"
	@touch "$$@"

$$(LAVG_DEFAULT_$(1)).csv: $$(SUMMARY_DEFAULT_$(1)).csv $$(SGB_DEFAULT_$(1)).csv
	@chmod ugo+x ./bin/compareAvg.awk
	# Generate lambda files for all alphas
	@set -Eeuo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		summary_src="$$(call GET,SUMMARY_TPL,$(1))"; sgb_src="$$(call GET,SGB_TPL,$(1))"; \
		summary_src="$$$${summary_src//-=ALPHA=-/$$$$a}"; sgb_src="$$$${sgb_src//-=ALPHA=-/$$$$a}"; \
		summary_src="$$$${summary_src//-=FORMAT=-/full}"; sgb_src="$$$${sgb_src//-=FORMAT=-/full}"; \
		[ -r "$$$$summary_src.csv" ] || (echo "Failed to find $$$$summary_src.csv" >&2; exit 1) ; \
		[ -r "$$$$sgb_src.csv" ] || (echo "Failed to find $$$$sgb_src.csv" >&2; exit 1) ; \
		lavg_dst="$$(call GET,LAVG_TPL,$(1))"; \
		lavg_dst="$$$${lavg_dst//-=ALPHA=-/$$$$a}"; \
		./bin/compareAvg.awk "$$$$summary_src.csv" "$$$$sgb_src.csv" > "$$$$lavg_dst.csv"; \
	done

# Lambda CSVs - copy from alpha-0.5 versions
$$(LAVG_$(1)).csv: $$(LAVG_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

$$(LMIN_DEFAULT_$(1)).csv: $$(SUMMARY_DEFAULT_$(1)).csv $$(SGB_DEFAULT_$(1)).csv
	@chmod ugo+x ./bin/compareMin.awk
	# Generate lambda files for all alphas
	@set -Eeuo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		summary_src="$$(call GET,SUMMARY_TPL,$(1))"; sgb_src="$$(call GET,SGB_TPL,$(1))"; \
		summary_src="$$$${summary_src//-=ALPHA=-/$$$$a}"; sgb_src="$$$${sgb_src//-=ALPHA=-/$$$$a}"; \
		summary_src="$$$${summary_src//-=FORMAT=-/full}"; sgb_src="$$$${sgb_src//-=FORMAT=-/full}"; \
		[ -r "$$$$summary_src.csv" ] || (echo "Failed to find $$$$summary_src.csv" >&2; exit 1) ; \
		[ -r "$$$$sgb_src.csv" ] || (echo "Failed to find $$$$sgb_src.csv" >&2; exit 1) ; \
		lmin_dst="$$(call GET,LMIN_TPL,$(1))"; \
		lmin_dst="$$$${lmin_dst//-=ALPHA=-/$$$$a}"; \
		./bin/compareMin.awk "$$$$summary_src.csv" "$$$$sgb_src.csv" > "$$$$lmin_dst.csv"; \
	done

$$(LMIN_$(1)).csv: $$(LMIN_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

$$(LMAX_DEFAULT_$(1)).csv: $$(SUMMARY_DEFAULT_$(1)).csv $$(SGB_DEFAULT_$(1)).csv
	@chmod ugo+x ./bin/compareMax.awk
	# Generate lambda files for all alphas
	@set -Eeuo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		summary_src="$$(call GET,SUMMARY_TPL,$(1))"; sgb_src="$$(call GET,SGB_TPL,$(1))"; \
		summary_src="$$$${summary_src//-=ALPHA=-/$$$$a}"; sgb_src="$$$${sgb_src//-=ALPHA=-/$$$$a}"; \
		summary_src="$$$${summary_src//-=FORMAT=-/full}"; sgb_src="$$$${sgb_src//-=FORMAT=-/full}"; \
		[ -r "$$$$summary_src.csv" ] || (echo "Failed to find $$$$summary_src.csv" >&2; exit 1) ; \
		[ -r "$$$$sgb_src.csv" ] || (echo "Failed to find $$$$sgb_src.csv" >&2; exit 1) ; \
		lmax_dst="$$(call GET,LMAX_TPL,$(1))"; \
		lmax_dst="$$$${lmax_dst//-=ALPHA=-/$$$$a}"; \
		./bin/compareMax.awk "$$$$summary_src.csv" "$$$$sgb_src.csv" > "$$$$lmax_dst.csv"; \
	done

$$(LMAX_$(1)).csv: $$(LMAX_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

$$(LSAVG_DEFAULT_$(1)).csv: $$(LAVG_DEFAULT_$(1)).csv
	@chmod ugo+x ./bin/lambdaStats.awk; \
	# Generate lambda files for all alphas
	@set -Eeuo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		lavg_dst="$$(call GET,LAVG_TPL,$(1))"; \
		lavg_dst="$$$${lavg_dst//-=ALPHA=-/$$$$a}"; \
		[ -r "$$$$lavg_dst.csv" ] || (echo "Failed to find $$$$lavg_dst.csv" >&2; exit 1) ; \
		./bin/lambdaStats.awk "$$$$lavg_dst.csv" > "$$$${lavg_dst/lambdaavg/lambdastatsavg}.csv"; \
	done

# Lambda Stats CSVs - copy from alpha-0.5 versions
$$(LSAVG_$(1)).csv: $$(LSAVG_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

$$(LSMIN_DEFAULT_$(1)).csv: $$(LMIN_DEFAULT_$(1)).csv
	@chmod ugo+x ./bin/lambdaStats.awk; \
	# Generate lambda files for all alphas
	@set -Eeuo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		lmin_dst="$$(call GET,LMIN_TPL,$(1))"; \
		lmin_dst="$$$${lmin_dst//-=ALPHA=-/$$$$a}"; \
		[ -r "$$$$lmin_dst.csv" ] || (echo "Failed to find $$$$lmin_dst.csv" >&2; exit 1) ; \
		./bin/lambdaStats.awk "$$$$lmin_dst.csv" > "$$$${lmin_dst/lambdamin/lambdastatsmin}.csv"; \
	done

$$(LSMIN_$(1)).csv: $$(LSMIN_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

$$(LSMAX_DEFAULT_$(1)).csv: $$(LMAX_DEFAULT_$(1)).csv
	@chmod ugo+x ./bin/lambdaStats.awk; \
	# Generate lambda files for all alphas
	@set -Eeuo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		lmax_dst="$$(call GET,LMAX_TPL,$(1))"; \
		lmax_dst="$$$${lmax_dst//-=ALPHA=-/$$$$a}"; \
		[ -r "$$$$lmax_dst.csv" ] || (echo "Failed to find $$$$lmax_dst.csv" >&2; exit 1) ; \
		./bin/lambdaStats.awk "$$$$lmax_dst.csv" > "$$$${lmax_dst/lambdamax/lambdastatsmax}.csv"; \
	done

$$(LSMAX_$(1)).csv: $$(LSMAX_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

# --- Verifications ---

# SUMMARY verify uses validator + SHA
$$(SUMMARY_$(1)).csv.verify: $(VALIDATESUMMARY) $(BITMAP) $(RAW) $$(SUMMARY_$(1)).csv | $(OUT)
	@set -Eeuo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		echo "Validating alpha-$$$$a empirical $$(call SFX,$(1))..."; \
		file="$$(call GET,SUMMARY_TPL,$(1)).csv"; \
		file="$$$${file//-=ALPHA=-/$$$$a}"; \
		file="$$$${file//-=FORMAT=-/full}"; \
		$(VALIDATESUMMARY) --model empirical --compat "$(COMPAT)" \
			--alpha "$$$$a" --file "$$$$file" --bitmap "$(BITMAP)" --raw "$(RAW)" | \
		tee "$$$$file.verify" && \
		(echo -n sha256= && sha256sum < "$$$$file" | awk '{print $$$$1}') | \
		tee -a "$$$$file.verify" || exit 1; \
	done
	cp "$$(call GET,SUMMARY_DEFAULT,$(1)).csv.verify" "$$(call GET,SUMMARY,$(1)).csv.verify"
	@touch "$$(call GET,SUMMARY,$(1)).csv.verify"

$$(SGB_$(1)).csv.verify: $(VALIDATESUMMARY) $(BITMAP) $(RAW) $$(SGB_$(1)).csv | $(OUT)
	@set -Eeuo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		echo "Validating alpha-$$$$a hl-a $$(call SFX,$(1))..."; \
		file="$$(call GET,SGB_TPL,$(1)).csv"; \
		file="$$$${file//-=ALPHA=-/$$$$a}"; \
		file="$$$${file//-=FORMAT=-/full}"; \
		$(VALIDATESUMMARY) --tolerance 0.5 --model hl-a --compat "$(COMPAT)" \
			--alpha "$$$$a" --file "$$$$file" --bitmap "$(BITMAP)" --raw "$(RAW)" | \
		tee "$$$$file.verify" && \
		(echo -n sha256= && sha256sum < "$$$$file" | awk '{print $$$$1}') | \
		tee -a "$$$$file.verify" || exit 1; \
	done
	cp "$$(call GET,SGB_DEFAULT,$(1)).csv.verify" "$$(call GET,SGB,$(1)).csv.verify"
	@touch "$$(call GET,SGB,$(1)).csv.verify"

# sha256-only verifies use a single pattern rule (see below)
# We keep them here for dependency wiring:
# Verify rules - skip MEDIUM/LARGE when SKIP_SUMMARY_ML is not set
$$(JOIN_VERIFY_$(1)):    $$(JOIN_$(1)).csv
	sha256sum "$$(call GET,JOIN,$(1)).csv" | tee "$$@"

$$(CPSLB_VERIFY_$(1)):  $$(CPSLB_$(1)).csv
	sha256sum "$$(call GET,CPSLB,$(1)).csv" | tee "$$@"

$$(CPS_SUMMARY_VERIFY_$(1)):  $$(CPS_SUMMARY_$(1)).csv
	sha256sum "$$(call GET,CPS_SUMMARY,$(1)).csv" | tee "$$@"

$$(LAVG_VERIFY_$(1)):   $$(LAVG_$(1)).csv
	sha256sum "$$(call GET,LAVG,$(1)).csv" | tee "$$@"

$$(LMIN_VERIFY_$(1)):   $$(LMIN_$(1)).csv
	sha256sum "$$(call GET,LMIN,$(1)).csv" | tee "$$@"

$$(LMAX_VERIFY_$(1)):   $$(LMAX_$(1)).csv
	sha256sum "$$(call GET,LMAX,$(1)).csv" | tee "$$@"

$$(LSAVG_VERIFY_$(1)):   $$(LSAVG_$(1)).csv
	sha256sum "$$(call GET,LSAVG,$(1)).csv" | tee "$$@"

$$(LSMIN_VERIFY_$(1)):   $$(LSMIN_$(1)).csv
	sha256sum "$$(call GET,LSMIN,$(1)).csv" | tee "$$@"

$$(LSMAX_VERIFY_$(1)):   $$(LSMAX_$(1)).csv
	sha256sum "$$(call GET,LSMAX,$(1)).csv" | tee "$$@"

# --- Cleaning

clean-$$(SFX_$(1)): 
	@test ! -d "$(OUT)" || $(RM) "$(OUT)"/*-$$(call SFX,$(1))*-$(COMPAT).csv.{verify,sha256}
	@test ! -d "$(OUT)" || $(RM) "$(OUT)"/*-$$(call SFX,$(1))-$(COMPAT).csv.{verify,sha256}
	@test ! -d "$(OUT)" || $(RM) "$(OUT)"/*-$$(call SFX,$(1))-$(COMPAT).csv
	@test ! -d "$(OUT)" || $(RM) "$(OUT)"/*-$$(call GET,CPS_SUMMARY_DEFAULT,$(1))-$(COMPAT).csv{,.verify,.sha256}
	@for a in $(ALPHAS); do for dst in $(call GET,LAVG_TPL,$(1)) $(call GET,LMIN_TPL,$(1)) $(call GET,LMAX_TPL,$(1)) $(call GET,LSAVG_TPL,$(1)) $(call GET,LSMIN_TPL,$(1)) $(call GET,LSMAX_TPL,$(1)) $(call GET,CPSLB_TPL,$(1)); do \
		$(RM) "$$$${dst//-=ALPHA=-/$$$$a}".csv{,.sha256}; \
	done; done
	@for a in $(ALPHAS); do for fmt in $(FORMATS_EMP); do for dst in $(call GET,SUMMARY_TPL,$(1)) $(call GET,SGB_TPL,$(1)); do \
		dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
		dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
		$(RM) "$$$$dst.csv"{,.verify,.sha256}; \
	done; done; done

clobber-$$(SFX_$(1)): clean-$$(SFX_$(1))
	for a in $(ALPHAS); do for fmt in $(FORMATS_EMP); do for dst in $(call GET,CPS_SUMMARY,$(1)) $(call GET,SUMMARY_TPL,$(1)) $(call GET,SGB_TPL,$(1)); do \
		dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
		dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
		sources=($(2)); \
		for suffix in $$(call SFX,$(1)A) $$(call SFX,$(1)B) $$(call SFX,$(1)C) $$(call SFX,$(1)D) $$(call SFX,$(1)E) $$(call SFX,$(1)F) ; do \
			sources+=("$$$${dst/-$$(call SFX,$(1))-/-$$$$suffix-}.partial.csv") ; \
		done; \
		echo $(RM) "$$$${sources[@]}" "$$$$dst.csv"; \
		$(RM) "$$$${sources[@]}" "$$$$dst.csv"; \
	done; done; done

touch-$$(SFX_$(1)): 
	@for a in $(ALPHAS); do for fmt in $(FORMATS_EMP); do for dst in $(call GET,CPS_SUMMARY_TPL,$(1)) $(call GET,SUMMARY_TPL,$(1)) $(call GET,SGB_TPL,$(1)); do \
		dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
		dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
		sources=($(2)); \
		for suffix in $$(call SFX,$(1)A) $$(call SFX,$(1)B) $$(call SFX,$(1)C) $$(call SFX,$(1)D) $$(call SFX,$(1)E) $$(call SFX,$(1)F) ; do \
			sources+=("$$$${dst/-$$(call SFX,$(1))-/-$$$$suffix-}.partial.csv") ; \
		done; \
		$(touch) "$$$${sources[@]}" "$$$$dst.csv"; \
	done; done; done

.PHONY: clean-$$(SFX_$(1)) clobber-$$(SFX_$(1)) touch-$$(SFX_$(1))

endef

# Expand the per-size template for all sizes
$(foreach SZ,$(SIZES),$(eval $(call SIZE_TEMPLATE3,$(SZ))))

# Fast summary counts via copy+append using
# - SMALL: generate with header up to END_SMALL
# - MEDIUM: depend on SMALL; copy SMALL header, then append from END_SMALL..END_MEDIUM
# - LARGE:  depend on MEDIUM; copy MEDIUM header, then append from END_MEDIUM..END_LARGE
# - If SKIP_SUMMARY_ML=1 and SIZE is MEDIUM/LARGE: DO NOT define a rule (read data/)

# SMALL
define PARTS_TEMPLATE

$$(call GET,SUMMARY_DEFAULT,$(1)).partial.csv: $(SUMMARY_BIN) $(RAW) | $(OUT) 
	@set -Eeuo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do mkdir -p "$(OUT)/alpha-$$$$a"; done ; \
	trace=(); \
	dec=(); \
	if [ -n "$$(call GET,SUMMARY_TPL,$(1))" ]; then \
		trace=( --trace=decade ); \
		dec=( --dec-n-start $$(call GET,START,$(1)) --dec-n-end $$(call GET,END,$(1)) \
			--dec-out="$$(call GET,SUMMARY_TPL,$(1)).partial.csv" \
			--dec-cps-summary="$$(call GET,CPS_SUMMARY,$(1)).partial.csv" ); \
	fi ; \
	prim=(); \
	if [ -n "$$(call GET,SUMMARY_TPL,$(2))" ]; then \
		trace=( --trace=primorial ); \
		prim=( --prim-n-start $$(call GET,START,$(2)) --prim-n-end $$(call GET,END,$(2)) \
			--prim-out="$$(call GET,SUMMARY_TPL,$(2)).partial.csv" \
 			--prim-cps-summary="$$(call GET,CPS_SUMMARY,$(2)).partial.csv" ); \
	fi ; \
	./$(SUMMARY_BIN) $(ALPHA_ARGS) --compat=$(COMPAT) --model=empirical \
		"$$$${dec[@]}" "$$$${prim[@]}"  "$$$${trace[@]}" "$(RAW)"

$$(call GET,SUMMARY_DEFAULT,$(2)).partial.csv: $$(call GET,SUMMARY_DEFAULT,$(1)).partial.csv
	@true

$$(call GET,CPS_SUMMARY,$(1)).partial.csv: $$(call GET,SUMMARY_DEFAULT,$(1)).partial.csv
	@true

$$(call GET,CPS_SUMMARY,$(2)).partial.csv: $$(call GET,SUMMARY_DEFAULT,$(2)).partial.csv
	@true

$$(call GET,SGB_DEFAULT,$(1)).partial.csv: $(SUMMARY_BIN) $(RAW) | $(OUT)
	@set -Eeuo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do mkdir -p "$(OUT)/alpha-$$$$a"; done ; \
	trace=(); \
	dec=(); \
	if [ -n "$$(call GET,SGB_TPL,$(1))" ]; then \
		trace=( --trace=decade ); \
		dec=( --dec-n-start $$(call GET,START,$(1)) --dec-n-end $$(call GET,END,$(1)) \
			--dec-out="$$(call GET,SGB_TPL,$(1)).partial.csv" ); \
	fi ; \
	if [ -n "$$(call GET,SGB_TPL,$(2))" ]; then \
		trace=( --trace=primorial ); \
		prim=( --prim-n-start $$(call GET,START,$(2)) --prim-n-end $$(call GET,END,$(2)) \
			--prim-out="$$(call GET,SGB_TPL,$(2)).partial.csv" ); \
	fi ; \
	./$(SUMMARY_BIN) $(ALPHA_ARGS) --compat=$(COMPAT) --model=hl-a \
		"$$$${dec[@]}" "$$$${prim[@]}"  "$$$${trace[@]}" "$(RAW)"


$$(call GET,SGB_DEFAULT,$(2)).partial.csv: $(call GET,SGB_DEFAULT,$(1)).partial.csv
	@true

endef

# SMALL

define PARTS_TEMPLATE2


$$(SUMMARY_DEFAULT_$(1)).csv: \
        $(foreach part,$($(1)PARTS),$$(SUMMARY_DEFAULT_$(1)$(part)).partial.csv) \
        $(call IFSIZE,$(2),$(foreach part,$($(2)PARTS),$$(SUMMARY_DEFAULT_$(2)$(part)).partial.csv)) \
        $(call IFSIZE,$(3),$(foreach part,$($(3)PARTS),$$(SUMMARY_DEFAULT_$(3)$(part)).partial.csv)) 
	@chmod ugo+x ./bin/mergeCPSLowerBound.awk
	@set -Eeuo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do for fmt in $(FORMATS_EMP); do \
		sources=(); \
		if [ -n "$(3)" ]; then \
			dst="$$(call GET,SUMMARY_TPL,$(3))"; \
			dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
			dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
			for suffix in $$(call SFX,$(3)A) $$(call SFX,$(3)B) $$(call SFX,$(3)C) $$(call SFX,$(3)D) $$(call SFX,$(3)E) $$(call SFX,$(3)F) ; do \
				sources+=("$$$${dst/-$$(call SFX,$(3))-/-$$$$suffix-}.partial.csv") ; \
			done; \
		fi; \
		if [ -n "$(2)" ]; then \
			dst="$$(call GET,SUMMARY_TPL,$(2))"; \
			dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
			dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
			for suffix in $$(call SFX,$(2)A) $$(call SFX,$(2)B) $$(call SFX,$(2)C) $$(call SFX,$(2)D) $$(call SFX,$(2)E) $$(call SFX,$(2)F) ; do \
				sources+=("$$$${dst/-$$(call SFX,$(2))-/-$$$$suffix-}.partial.csv") ; \
			done; \
		fi; \
		dst="$$(call GET,SUMMARY_TPL,$(1))"; \
		dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
		dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
		for suffix in $$(call SFX,$(1)A) $$(call SFX,$(1)B) $$(call SFX,$(1)C) $$(call SFX,$(1)D) $$(call SFX,$(1)E) $$(call SFX,$(1)F) ; do \
			sources+=("$$$${dst/-$$(call SFX,$(1))-/-$$$$suffix-}.partial.csv") ; \
		done; \
		if [ -n "$$$${sources[*]}" ]; then  \
			if [ "cps" = "$$$$fmt" ]; then \
				./bin/mergeCPSLowerBound.awk "$$$${sources[@]}" > "$$$$dst.csv" ; \
			else \
				(head -1 "$$$${sources[0]}";for s in "$$$${sources[@]}"; do tail -n +2 "$$$$s"; done) > "$$$$dst.csv"; \
			fi; \
		fi; \
	done; done
	@touch "$$@"

$$(SGB_DEFAULT_$(1)).csv: \
        $(foreach part,$($(1)PARTS),$$(SGB_DEFAULT_$(1)$(part)).partial.csv) \
        $(call IFSIZE,$(2),$(foreach part,$($(2)PARTS),$$(SGB_DEFAULT_$(2)$(part)).partial.csv)) \
        $(call IFSIZE,$(3),$(foreach part,$($(3)PARTS),$$(SGB_DEFAULT_$(3)$(part)).partial.csv)) 
	set -Eeuo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do for fmt in $(FORMATS_EMP); do \
		sources=(); \
		if [ -n "$(3)" ]; then \
			dst="$$(call GET,SGB_TPL,$(3))"; \
			dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
			dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
			for suffix in $$(call SFX,$(3)A) $$(call SFX,$(3)B) $$(call SFX,$(3)C) $$(call SFX,$(3)D) $$(call SFX,$(3)E) $$(call SFX,$(3)F) ; do \
				sources+=("$$$${dst/-$$(call SFX,$(3))-/-$$$$suffix-}.partial.csv") ; \
			done; \
		fi; \
		if [ -n "$(2)" ]; then \
			dst="$$(call GET,SGB_TPL,$(2))"; \
			dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
			dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
			for suffix in $$(call SFX,$(2)A) $$(call SFX,$(2)B) $$(call SFX,$(2)C) $$(call SFX,$(2)D) $$(call SFX,$(2)E) $$(call SFX,$(2)F) ; do \
				sources+=("$$$${dst/-$$(call SFX,$(2))-/-$$$$suffix-}.partial.csv") ; \
			done; \
		fi; \
		dst="$$(call GET,SGB_TPL,$(1))"; \
		dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
		dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
		for suffix in $$(call SFX,$(1)A) $$(call SFX,$(1)B) $$(call SFX,$(1)C) $$(call SFX,$(1)D) $$(call SFX,$(1)E) $$(call SFX,$(1)F) ; do \
			sources+=("$$$${dst/-$$(call SFX,$(1))-/-$$$$suffix-}.partial.csv") ; \
		done; \
		if [ -n "$$$${sources[*]}" ] && [ "cps" != "$$$$fmt" ] ; then  \
			(head -1 "$$$${sources[0]}";for s in "$$$${sources[@]}"; do tail -n +2 "$$$$s"; done) > "$$$$dst.csv"; \
		fi; \
	done; done
	@touch "$$@"

endef

#SMALL
$(foreach PRT,$(SMALLPARTS),$(eval $(call SUMMARY_TEMPLATE,SMALL$(PRT))))

CPS_SUMMARY_SMALL_PARTS := $(CPS_SUMMARY_SMALLA).partial.csv $(CPS_SUMMARY_SMALLB).partial.csv $(CPS_SUMMARY_SMALLC).partial.csv $(CPS_SUMMARY_SMALLD).partial.csv $(CPS_SUMMARY_SMALLE).partial.csv

$(foreach PRT,$(SMALLPARTS),$(eval $(call SUMMARY_TEMPLATE,SPRIM$(PRT))))

CPS_SUMMARY_SPRIM_PARTS := $(CPS_SUMMARY_SPRIMA).partial.csv $(CPS_SUMMARY_SPRIMB).partial.csv $(CPS_SUMMARY_SPRIMC).partial.csv $(CPS_SUMMARY_SPRIMD).partial.csv $(CPS_SUMMARY_SPRIME).partial.csv

$(foreach PRT,$(SMALLPARTS),$(eval $(call PARTS_TEMPLATE,SMALL$(PRT),SPRIM$(PRT),SMALL,SPRIM)))

$(eval $(call PARTS_TEMPLATE2,SMALL,,))

$(CPS_SUMMARY_SMALL).csv: $(SUMMARY_DEFAULT_SMALL).csv | $(MERGECPS)
	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	$(MERGECPS) $(foreach a,$(CPS_SUMMARY_SMALL_PARTS),--input $(a)) --output "$@"

$(eval $(call PARTS_TEMPLATE2,SPRIM,,))

$(CPS_SUMMARY_SPRIM).csv: $(SUMMARY_DEFAULT_SPRIM).csv | $(MERGECPS)
	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	$(MERGECPS) $(foreach a,$(CPS_SUMMARY_SPRIM_PARTS),--input $(a)) --output "$@"

# MEDIUM
$(foreach PRT,$(MEDIUMPARTS),$(eval $(call SUMMARY_TEMPLATE,MEDIUM$(PRT))))

CPS_SUMMARY_MEDIUM_PARTS := $(CPS_SUMMARY_MEDIUMA).partial.csv $(CPS_SUMMARY_MEDIUMB).partial.csv $(CPS_SUMMARY_MEDIUMC).partial.csv $(CPS_SUMMARY_MEDIUMD).partial.csv $(CPS_SUMMARY_MEDIUME).partial.csv

$(foreach PRT,$(MEDIUMPARTS),$(eval $(call SUMMARY_TEMPLATE,MPRIM$(PRT))))

CPS_SUMMARY_MPRIM_PARTS := $(CPS_SUMMARY_MPRIMA).partial.csv $(CPS_SUMMARY_MPRIMB).partial.csv $(CPS_SUMMARY_MPRIMC).partial.csv $(CPS_SUMMARY_MPRIMD).partial.csv $(CPS_SUMMARY_MPRIME).partial.csv

$(foreach PRT,$(MEDIUMPARTS),$(eval $(call PARTS_TEMPLATE,MEDIUM$(PRT),MPRIM$(PRT),MEDIUM,MPRIM)))

$(eval $(call PARTS_TEMPLATE2,MEDIUM,SMALL,))

$(CPS_SUMMARY_MEDIUM).csv: $(CPS_SUMMARY_SMALL).csv $(SUMMARY_DEFAULT_MEDIUM).csv | $(MERGECPS)
	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	$(MERGECPS) $(foreach a,$(CPS_SUMMARY_SMALL).csv $(CPS_SUMMARY_MEDIUM_PARTS),--input $(a)) --output "$@"

$(eval $(call PARTS_TEMPLATE2,MPRIM,SPRIM,))

$(CPS_SUMMARY_MPRIM).csv: $(CPS_SUMMARY_SPRIM).csv $(SUMMARY_DEFAULT_MPRIM).csv | $(MERGECPS)
	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	$(MERGECPS) $(foreach a,$(CPS_SUMMARY_SPRIM).csv $(CPS_SUMMARY_MPRIM_PARTS),--input $(a)) --output "$@"

# LARGE

$(foreach PRT,$(LARGEPARTS),$(eval $(call SUMMARY_TEMPLATE,LARGE$(PRT))))

CPS_SUMMARY_LARGE_PARTS := $(CPS_SUMMARY_LARGEA).partial.csv $(CPS_SUMMARY_LARGEB).partial.csv $(CPS_SUMMARY_LARGEC).partial.csv $(CPS_SUMMARY_LARGED).partial.csv $(CPS_SUMMARY_LARGEF).partial.csv

$(foreach PRT,$(LARGEPARTS),$(eval $(call SUMMARY_TEMPLATE,LPRIM$(PRT))))

CPS_SUMMARY_LPRIM_PARTS := $(CPS_SUMMARY_LPRIMA).partial.csv $(CPS_SUMMARY_LPRIMB).partial.csv $(CPS_SUMMARY_LPRIMC).partial.csv $(CPS_SUMMARY_LPRIMD).partial.csv $(CPS_SUMMARY_LPRIME).partial.csv

$(foreach PRT,$(LARGEPARTS),$(eval $(call PARTS_TEMPLATE,LARGE$(PRT),LPRIM$(PRT),LARGE,LPRIM)))

$(eval $(call PARTS_TEMPLATE2,LARGE,MEDIUM,SMALL))

$(CPS_SUMMARY_LARGE).csv: $(CPS_SUMMARY_MEDIUM).csv $(SUMMARY_DEFAULT_LARGE).csv | $(MERGECPS)
	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	$(MERGECPS) $(foreach a,$(CPS_SUMMARY_MEDIUM).csv $(CPS_SUMMARY_LARGE_PARTS),--input $(a)) --output "$@"

$(eval $(call PARTS_TEMPLATE2,LPRIM,MPRIM,SPRIM))

$(CPS_SUMMARY_LPRIM).csv: $(CPS_SUMMARY_MPRIM).csv $(SUMMARY_DEFAULT_LPRIM).csv | $(MERGECPS)
	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	$(MERGECPS) $(foreach a,$(CPS_SUMMARY_MPRIM).csv $(CPS_SUMMARY_LPRIM_PARTS),--input $(a)) --output "$@"


# # Predicted HL-A pairs
# $(SGB_DEFAULT_SMALL).csv: $(SUMMARY_BIN) $(RAW) | $(OUT)
# 	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
# 	for a in $(ALPHAS); do mkdir -p "$(OUT)/alpha-$$a"; done
# 	./$(SUMMARY_BIN) $(ALPHA_ARGS) --compat=$(COMPAT) --model=hl-a \
# 	--dec-n-start $(START_SMALL) --dec-n-end $(END_SMALL) \
# 	--prim-n-start $(START_SPRIM) --prim-n-end $(END_SPRIM) \
# 	--trace=primorial \
# 	--dec-out="$(SGB_TPL_SMALL).csv" --prim-out="$(SGB_TPL_SPRIM).csv" "$(RAW)"
# 	@touch "$@"
# 
# $(SGB_DEFAULT_SPRIM).csv: $(SGB_DEFAULT_SMALL).csv
# 	@true
# 
# $(SGB_DEFAULT_MEDIUM).csv: $(SGB_DEFAULT_SMALL).csv $(SGB_DEFAULT_MEDIUM).partial.csv
# 	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
# 	for a in $(ALPHAS); do for fmt in $(FORMATS_HL_A); do \
# 	  src="$(SGB_TPL_SMALL)"; dst="$(SGB_TPL_MEDIUM)"; \
# 	  src="$${src//-=ALPHA=-/$$a}"; dst="$${dst//-=ALPHA=-/$$a}"; \
# 	  src="$${src//-=FORMAT=-/$$fmt}"; dst="$${dst//-=FORMAT=-/$$fmt}"; \
# 	  ( dir="$$dst"; dir="$${dir%/*}"; [ -d "$$dir" ] || mkdir -p "$$dir" ) ; \
# 	  if [ -r "$$dst.partial.csv" ]; then (cat "$$src.csv"; tail -n +2 "$$dst.partial.csv") > "$$dst.csv"; fi; \
# 	done; done
# 
# $(SGB_DEFAULT_MPRIM).csv: $(SGB_DEFAULT_SPRIM).csv $(SGB_DEFAULT_MEDIUM).partial.csv
# 	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
# 	for a in $(ALPHAS); do for fmt in $(FORMATS_HL_A); do \
# 	  src="$(SGB_TPL_SPRIM)"; dst="$(SGB_TPL_MPRIM)"; \
# 	  src="$${src//-=ALPHA=-/$$a}"; dst="$${dst//-=ALPHA=-/$$a}"; \
# 	  src="$${src//-=FORMAT=-/$$fmt}"; dst="$${dst//-=FORMAT=-/$$fmt}"; \
# 	  ( dir="$$dst"; dir="$${dir%/*}"; [ -d "$$dir" ] || mkdir -p "$$dir" ) ; \
# 	  if [ -r "$$dst.partial.csv" ]; then (cat "$$src.csv"; tail -n +2 "$$dst.partial.csv") > "$$dst.csv"; fi; \
# 	done; done
# 
# 
# $(SGB_DEFAULT_LARGE).csv: $(SGB_DEFAULT_MEDIUM).csv $(SGB_DEFAULT_LARGE).partial.csv
# 	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
# 	for a in $(ALPHAS); do for fmt in $(FORMATS_HL_A); do \
# 		src="$(SGB_TPL_MEDIUM)"; dst="$(SGB_TPL_LARGE)"; \
# 		src="$${src//-=ALPHA=-/$$a}"; dst="$${dst//-=ALPHA=-/$$a}"; \
# 		src="$${src//-=FORMAT=-/$$fmt}"; dst="$${dst//-=FORMAT=-/$$fmt}"; \
# 		( dir="$$dst"; dir="$${dir%/*}"; [ -d "$$dir" ] || mkdir -p "$$dir" ) ; \
# 		if [ -r "$$dst.partial.csv" ]; then (cat "$$src.csv"; tail -n +2 "$$dst.partial.csv") > "$$dst.csv"; fi; \
# 	done; done
# 
# $(SGB_DEFAULT_LPRIM).csv: $(SGB_DEFAULT_MPRIM).csv $(SGB_DEFAULT_LARGE).partial.csv
# 	@set -Eeuo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
# 	for a in $(ALPHAS); do for fmt in $(FORMATS_HL_A); do \
# 		src="$(SGB_TPL_MPRIM)"; dst="$(SGB_TPL_LPRIM)"; \
# 		src="$${src//-=ALPHA=-/$$a}"; dst="$${dst//-=ALPHA=-/$$a}"; \
# 		src="$${src//-=FORMAT=-/$$fmt}"; dst="$${dst//-=FORMAT=-/$$fmt}"; \
# 		( dir="$$dst"; dir="$${dir%/*}"; [ -d "$$dir" ] || mkdir -p "$$dir" ) ; \
# 		if [ -r "$$dst.partial.csv" ]; then (cat "$$src.csv"; tail -n +2 "$$dst.partial.csv") > "$$dst.csv"; fi; \
# 	done; done

# ---------- Generic sha256 rule ----------
%.sha256: %
	sha256sum "$<" | tee "$@"

# ---------- Top-level generation groups ----------
generate:       $(BITMAP) $(RAW) $(GBP) $(SGB_SMALL).csv $(SGB_SPRIM).csv $(SUMMARY_SMALL).csv \
	$(JOIN_SMALL).csv $(CPSLB_SMALL).csv $(CPSLB_SPRIM).csv $(LAVG_SMALL).csv $(LMIN_SMALL).csv \
	$(LMAX_SMALL).csv $(LSAVG_SMALL).csv $(LSMIN_SMALL).csv $(LSMAX_SMALL).csv

generate-medium: $(BITMAP) $(RAW) $(GBP) $(SGB_MEDIUM).csv $(SUMMARY_MEDIUM).csv \
	$(JOIN_MEDIUM).csv $(CPSLB_MEDIUM).csv $(CPSLB_MPRIM).csv  $(LAVG_MEDIUM).csv $(LMIN_MEDIUM).csv \
	$(LMAX_MEDIUM).csv $(LSAVG_MEDIUM).csv $(LSMIN_MEDIUM).csv $(LSMAX_MEDIUM).csv

generate-large: $(BITMAP) $(RAW) $(GBP) $(SGB_LARGE).csv $(SUMMARY_LARGE).csv \
	$(JOIN_LARGE).csv $(CPSLB_LARGE).csv $(CPSLB_LPRIM).csv $(LAVG_LARGE).csv $(LMIN_LARGE).csv \
	$(LMAX_LARGE).csv $(LSAVG_LARGE).csv $(LSMIN_LARGE).csv $(LSMAX_LARGE).csv

# ---------- Certification (base + per-size chains) ----------
BITMAP_VERIFY := $(BITMAP).verify
RAW_VERIFY    := $(RAW).verify
GBP_VERIFY    := $(GBP).verify

$(BITMAP_VERIFY): $(CERTIFYPRIMES) $(BITMAP) | $(OUT)
	./$(CERTIFYPRIMES) --bitmap --file "$(BITMAP)" | tee "$@"
	@(echo -n "sha256=" && sha256sum < "$(BITMAP)" | awk '{print $$1}') | tee -a "$@"

$(RAW_VERIFY): $(CERTIFYPRIMES) $(RAW) | $(OUT)
	./$(CERTIFYPRIMES) --binary --file "$(RAW)" | tee "$@"
	@(echo -n "sha256=" && sha256sum < "$(RAW)" | awk '{print $$1}') | tee -a "$@"

$(GBP_VERIFY): $(CERTIFYGBPAIRS) $(GBP) $(BITMAP) | $(OUT)
	./$(CERTIFYGBPAIRS) --bitmap "$(BITMAP)" --file "$(GBP)" | tee "$@"
	@(echo -n "sha256=" && sha256sum < "$(BITMAP)" | awk '{print $$1}') | tee -a "$@"

certify: $(BITMAP_VERIFY) $(RAW_VERIFY) $(GBP_VERIFY) \
	$(SGB_SMALL).csv.verify \
	$(SUMMARY_SMALL).csv.verify $(JOIN_VERIFY_SMALL) $(CPSLB_VERIFY_SMALL) $(CPS_SUMMARY_VERIFY_SMALL) \
	$(LAVG_VERIFY_SMALL) $(LMIN_VERIFY_SMALL) $(LMAX_VERIFY_SMALL) \
        $(LSAVG_VERIFY_SMALL) $(LSMIN_VERIFY_SMALL) $(LSMAX_VERIFY_SMALL) \
	$(SGB_SPRIM).csv.verify \
	$(SUMMARY_SPRIM).csv.verify $(JOIN_VERIFY_SPRIM) $(CPSLB_VERIFY_SPRIM) $(CPS_SUMMARY_VERIFY_SPRIM) \
	$(LAVG_VERIFY_SPRIM) $(LMIN_VERIFY_SPRIM) $(LMAX_VERIFY_SPRIM) \
        $(LSAVG_VERIFY_SPRIM) $(LSMIN_VERIFY_SPRIM) $(LSMAX_VERIFY_SPRIM)

certify-medium: \
	$(SGB_MEDIUM).csv.verify $(SUMMARY_MEDIUM).csv.verify \
	$(JOIN_VERIFY_MEDIUM) $(CPSLB_VERIFY_MEDIUM) $(CPS_SUMMARY_VERIFY_MEDIUM) \
	$(LAVG_VERIFY_MEDIUM) $(LMIN_VERIFY_MEDIUM) $(LMAX_VERIFY_MEDIUM) \
	$(LSAVG_VERIFY_MEDIUM) $(LSMIN_VERIFY_MEDIUM) $(LSMAX_VERIFY_MEDIUM) \
	$(SGB_MPRIM).csv.verify $(SUMMARY_MPRIM).csv.verify \
	$(JOIN_VERIFY_MPRIM) $(CPSLB_VERIFY_MPRIM) $(CPS_SUMMARY_VERIFY_MPRIM) \
	$(LAVG_VERIFY_MPRIM) $(LMIN_VERIFY_MPRIM) $(LMAX_VERIFY_MPRIM) \
	$(LSAVG_VERIFY_MPRIM) $(LSMIN_VERIFY_MPRIM) $(LSMAX_VERIFY_MPRIM) \
	certify

certify-large: \
	$(SGB_LARGE).csv.verify $(SUMMARY_LARGE).csv.verify \
	$(JOIN_VERIFY_LARGE) $(CPSLB_VERIFY_LARGE) $(CPS_SUMMARY_VERIFY_LARGE) \
	$(LAVG_VERIFY_LARGE) $(LMIN_VERIFY_LARGE) $(LMAX_VERIFY_LARGE) \
	$(LSAVG_VERIFY_LARGE) $(LSMIN_VERIFY_LARGE) $(LSMAX_VERIFY_LARGE) \
	$(SGB_LPRIM).csv.verify $(SUMMARY_LPRIM).csv.verify \
	$(JOIN_VERIFY_LPRIM) $(CPSLB_VERIFY_LPRIM) $(CPS_SUMMARY_VERIFY_LPRIM) \
	$(LAVG_VERIFY_LPRIM) $(LMIN_VERIFY_LPRIM) $(LMAX_VERIFY_LPRIM) \
	$(LSAVG_VERIFY_LPRIM) $(LSMIN_VERIFY_LPRIM) $(LSMAX_VERIFY_LPRIM) \
	certify-medium

# ---------- Compare against golden references ----------
.PHONY: verify verify-medium verify-large validate validate-medium validate-large

verify: certify 
	@cmp "$(BITMAP_VERIFY)" "$(BITMAP_GOLD)" && echo "Validated $(BITMAP)"
	@cmp "$(RAW_VERIFY)"    "$(RAW_GOLD)" && echo "Validated $(RAW)"
	@cmp "$(GBP_VERIFY)"    "$(GBP_GOLD)" && echo "Validated $(GBP)"
	@cmp "$(SGB_SMALL).csv.verify"    "$(SGB_GOLD_SMALL)" && echo "Validated $(SGB_SMALL).csv"
	@cmp "$(SUMMARY_SMALL).csv.verify" "$(SUMMARY_GOLD_SMALL)" && echo "Validated $(SUMMARY_SMALL).csv"
#	@cmp "$(CPS_SUMMARY_VERIFY_SMALL)" "$(CPS_SUMMARY_GOLD_SMALL)" && echo "Validated $(CPS_SUMMARY_SMALL).csv"
	@cmp "$(JOIN_VERIFY_SMALL)" "$(JOIN_GOLD_SMALL)" && echo "Validated $(JOIN_SMALL).csv"
	@cmp "$(CPSLB_VERIFY_SMALL)"   "$(CPSLB_GOLD_SMALL)" && echo "Validated $(CPSLB_SMALL).csv"
	@cmp "$(LAVG_VERIFY_SMALL)"    "$(LAVG_GOLD_SMALL)" && echo "Validated $(LAVG_SMALL).csv"
	@cmp "$(LMIN_VERIFY_SMALL)"    "$(LMIN_GOLD_SMALL)" && echo "Validated $(LMIN_SMALL).csv"
	@cmp "$(LMAX_VERIFY_SMALL)"    "$(LMAX_GOLD_SMALL)" && echo "Validated $(LMAX_SMALL).csv"
	@cmp "$(LSAVG_VERIFY_SMALL)"    "$(LSAVG_GOLD_SMALL)" && echo "Validated $(LSAVG_SMALL).csv"
	@cmp "$(LSMIN_VERIFY_SMALL)"    "$(LSMIN_GOLD_SMALL)" && echo "Validated $(LSMIN_SMALL).csv"
	@cmp "$(LSMAX_VERIFY_SMALL)"    "$(LSMAX_GOLD_SMALL)" && echo "Validated $(LSMAX_SMALL).csv"
	@cmp "$(SGB_SPRIM).csv.verify"    "$(SGB_GOLD_SPRIM)" && echo "Validated $(SGB_SPRIM).csv"
	@cmp "$(SUMMARY_SPRIM).csv.verify" "$(SUMMARY_GOLD_SPRIM)" && echo "Validated $(SUMMARY_SPRIM).csv"
#	@cmp "$(CPS_SUMMARY_VERIFY_SPRIM)" "$(CPS_SUMMARY_GOLD_SPRIM)" && echo "Validated $(CPS_SUMMARY_SPRIM).csv"
	@cmp "$(JOIN_VERIFY_SPRIM)" "$(JOIN_GOLD_SPRIM)" && echo "Validated $(JOIN_SPRIM).csv"
	@cmp "$(CPSLB_VERIFY_SPRIM)"   "$(CPSLB_GOLD_SPRIM)" && echo "Validated $(CPSLB_SPRIM).csv"
	@cmp "$(LAVG_VERIFY_SPRIM)"    "$(LAVG_GOLD_SPRIM)" && echo "Validated $(LAVG_SPRIM).csv"
	@cmp "$(LMIN_VERIFY_SPRIM)"    "$(LMIN_GOLD_SPRIM)" && echo "Validated $(LMIN_SPRIM).csv"
	@cmp "$(LMAX_VERIFY_SPRIM)"    "$(LMAX_GOLD_SPRIM)" && echo "Validated $(LMAX_SPRIM).csv"
	@cmp "$(LSAVG_VERIFY_SPRIM)"    "$(LSAVG_GOLD_SPRIM)" && echo "Validated $(LSAVG_SPRIM).csv"
	@cmp "$(LSMIN_VERIFY_SPRIM)"    "$(LSMIN_GOLD_SPRIM)" && echo "Validated $(LSMIN_SPRIM).csv"
	@cmp "$(LSMAX_VERIFY_SPRIM)"    "$(LSMAX_GOLD_SPRIM)" && echo "Validated $(LSMAX_SPRIM).csv"

verify-medium: $(OUT)/verify-medium-$(COMPAT).stamp
	@echo "Medium validation completed successfully!"

$(OUT)/verify-medium-$(COMPAT).stamp: certify-medium verify
	@cmp "$(SGB_MEDIUM).csv.verify"    "$(SGB_GOLD_MEDIUM)" && echo "Validated $(SGB_MEDIUM).csv"
	@cmp "$(SUMMARY_MEDIUM).csv.verify" "$(SUMMARY_GOLD_MEDIUM)" && echo "Validated $(SUMMARY_MEDIUM).csv"
#	@cmp "$(CPS_SUMMARY_VERIFY_MEDIUM)" "$(CPS_SUMMARY_GOLD_MEDIUM)" && echo "Validated $(CPS_SUMMARY_MEDIUM).csv"
	@cmp "$(JOIN_VERIFY_MEDIUM)"   "$(JOIN_GOLD_MEDIUM)" && echo "Validated $(JOIN_MEDIUM).csv"
	@cmp "$(CPSLB_VERIFY_MEDIUM)"   "$(CPSLB_GOLD_MEDIUM)" && echo "Validated $(CPSLB_MEDIUM).csv"
	@cmp "$(LAVG_VERIFY_MEDIUM)"    "$(LAVG_GOLD_MEDIUM)" && echo "Validated $(LAVG_MEDIUM).csv"
	@cmp "$(LMIN_VERIFY_MEDIUM)"    "$(LMIN_GOLD_MEDIUM)" && echo "Validated $(LMIN_MEDIUM).csv"
	@cmp "$(LMAX_VERIFY_MEDIUM)"    "$(LMAX_GOLD_MEDIUM)" && echo "Validated $(LMAX_MEDIUM).csv"
	@cmp "$(LSAVG_VERIFY_MEDIUM)"    "$(LSAVG_GOLD_MEDIUM)" && echo "Validated $(LSAVG_MEDIUM).csv"
	@cmp "$(LSMIN_VERIFY_MEDIUM)"    "$(LSMIN_GOLD_MEDIUM)" && echo "Validated $(LSMIN_MEDIUM).csv"
	@cmp "$(LSMAX_VERIFY_MEDIUM)"    "$(LSMAX_GOLD_MEDIUM)" && echo "Validated $(LSMAX_MEDIUM).csv"
	@cmp "$(SUMMARY_MPRIM).csv.verify" "$(SUMMARY_GOLD_MPRIM)" && echo "Validated $(SUMMARY_MPRIM).csv"
#	@cmp "$(CPS_SUMMARY_VERIFY_MPRIM)" "$(CPS_SUMMARY_GOLD_MPRIM)" && echo "Validated $(CPS_SUMMARY_MPRIM).csv"
	@cmp "$(JOIN_VERIFY_MPRIM)"   "$(JOIN_GOLD_MPRIM)" && echo "Validated $(JOIN_MPRIM).csv"
	@cmp "$(CPSLB_VERIFY_MPRIM)"   "$(CPSLB_GOLD_MPRIM)" && echo "Validated $(CPSLB_MPRIM).csv"
	@cmp "$(LAVG_VERIFY_MPRIM)"    "$(LAVG_GOLD_MPRIM)" && echo "Validated $(LAVG_MPRIM).csv"
	@cmp "$(LMIN_VERIFY_MPRIM)"    "$(LMIN_GOLD_MPRIM)" && echo "Validated $(LMIN_MPRIM).csv"
	@cmp "$(LMAX_VERIFY_MPRIM)"    "$(LMAX_GOLD_MPRIM)" && echo "Validated $(LMAX_MPRIM).csv"
	@cmp "$(LSAVG_VERIFY_MPRIM)"    "$(LSAVG_GOLD_MPRIM)" && echo "Validated $(LSAVG_MPRIM).csv"
	@cmp "$(LSMIN_VERIFY_MPRIM)"    "$(LSMIN_GOLD_MPRIM)" && echo "Validated $(LSMIN_MPRIM).csv"
	@cmp "$(LSMAX_VERIFY_MPRIM)"    "$(LSMAX_GOLD_MPRIM)" && echo "Validated $(LSMAX_MPRIM).csv"
	@touch "$@"

verify-large: $(OUT)/verify-large-$(COMPAT).stamp
	@echo "Large validation completed successfully!"

$(OUT)/verify-large-$(COMPAT).stamp: certify-large verify-medium
	@cmp "$(SGB_LARGE).csv.verify"    "$(SGB_GOLD_LARGE)" && echo "Validated $(SGB_LARGE).csv"
	@cmp "$(SUMMARY_LARGE).csv.verify" "$(SUMMARY_GOLD_LARGE)" && echo "Validated $(SUMMARY_LARGE).csv"
#	@cmp "$(CPS_SUMMARY_VERIFY_LARGE)" "$(CPS_SUMMARY_GOLD_LARGE)" && echo "Validated $(CPS_SUMMARY_LARGE).csv"
	@cmp "$(JOIN_VERIFY_LARGE)"   "$(JOIN_GOLD_LARGE)" && echo "Validated $(JOIN_LARGE).csv"
	@cmp "$(CPSLB_VERIFY_LARGE)"   "$(CPSLB_GOLD_LARGE)" && echo "Validated $(CPSLB_LARGE).csv"
	@cmp "$(LAVG_VERIFY_LARGE)"    "$(LAVG_GOLD_LARGE)" && echo "Validated $(LAVG_LARGE).csv"
	@cmp "$(LMIN_VERIFY_LARGE)"    "$(LMIN_GOLD_LARGE)" && echo "Validated $(LMIN_LARGE).csv"
	@cmp "$(LMAX_VERIFY_LARGE)"    "$(LMAX_GOLD_LARGE)" && echo "Validated $(LMAX_LARGE).csv"
	@cmp "$(LSAVG_VERIFY_LARGE)"    "$(LSAVG_GOLD_LARGE)" && echo "Validated $(LSAVG_LARGE).csv"
	@cmp "$(LSMIN_VERIFY_LARGE)"    "$(LSMIN_GOLD_LARGE)" && echo "Validated $(LSMIN_LARGE).csv"
	@cmp "$(LSMAX_VERIFY_LARGE)"    "$(LSMAX_GOLD_LARGE)" && echo "Validated $(LSMAX_LARGE).csv"
	@cmp "$(SUMMARY_LPRIM).csv.verify" "$(SUMMARY_GOLD_LPRIM)" && echo "Validated $(SUMMARY_LPRIM).csv"
#	@cmp "$(CPS_SUMMARY_VERIFY_LPRIM)" "$(CPS_SUMMARY_GOLD_LPRIM)" && echo "Validated $(CPS_SUMMARY_LPRIM).csv"
	@cmp "$(JOIN_VERIFY_LPRIM)"   "$(JOIN_GOLD_LPRIM)" && echo "Validated $(JOIN_LPRIM).csv"
	@cmp "$(CPSLB_VERIFY_LPRIM)"   "$(CPSLB_GOLD_LPRIM)" && echo "Validated $(CPSLB_LPRIM).csv"
	@cmp "$(LAVG_VERIFY_LPRIM)"    "$(LAVG_GOLD_LPRIM)" && echo "Validated $(LAVG_LPRIM).csv"
	@cmp "$(LMIN_VERIFY_LPRIM)"    "$(LMIN_GOLD_LPRIM)" && echo "Validated $(LMIN_LPRIM).csv"
	@cmp "$(LMAX_VERIFY_LPRIM)"    "$(LMAX_GOLD_LPRIM)" && echo "Validated $(LMAX_LPRIM).csv"
	@cmp "$(LSAVG_VERIFY_LPRIM)"    "$(LSAVG_GOLD_LPRIM)" && echo "Validated $(LSAVG_LPRIM).csv"
	@cmp "$(LSMIN_VERIFY_LPRIM)"    "$(LSMIN_GOLD_LPRIM)" && echo "Validated $(LSMIN_LPRIM).csv"
	@cmp "$(LSMAX_VERIFY_LPRIM)"    "$(LSMAX_GOLD_LPRIM)" && echo "Validated $(LSMAX_LPRIM).csv"
	@touch "$@"

validate: verify

validate-medium: verify-medium

validate-large: verify-large

# ==========================================================================


# ---------- Housekeeping ----------
.PHONY: all generate generate-medium generate-large clean clean-small clean-medium clean-large clobber clobber-small clobber-medium clobber-large  touch touch-small touch-mediam touch-large
 
clean: clean-small clean-medium clean-large
	@test ! -d "$(OUT)" || $(RM) "$(OUT)"/*.verify "$(OUT)"/*.sha256

clean-small: clean-$(SUFFIX_SMALL) clean-$(SUFFIX_SPRIM)

clean-medium: clean-$(SUFFIX_MEDIUM) clean-$(SUFFIX_MPRIM)

clean-large: clean-$(SUFFIX_LARGE) clean-$(SUFFIX_LPRIM)

ALL_OUTPUTS := $(BITMAP) $(RAW) $(GBP) \
               $(OUTPUT_SMALL) $(OUTPUT_MEDIUM) $(OUTPUT_LARGE)

clobber: clean clobber-small clobber-medium clobber-large
	$(RM) $(ALL_OUTPUTS)
	$(MAKE) -C src clean

clobber-small: clobber-$(SUFFIX_SMALL) clobber-$(SUFFIX_SPRIM)

clobber-medium: clobber-$(SUFFIX_MEDIUM) clobber-$(SUFFIX_MPRIM)

clobber-large: clobber-$(SUFFIX_LARGE) clobber-$(SUFFIX_LPRIM)

touch: touch-small touch-medium touch-large
	$(RM) $(ALL_OUTPUTS)
	$(MAKE) -C src clean

touch-small: touch-$(SUFFIX_SMALL) touch-$(SUFFIX_SPRIM)

touch-medium: touch-$(SUFFIX_MEDIUM) touch-$(SUFFIX_MPRIM)

touch-large: touch-$(SUFFIX_LARGE) touch-$(SUFFIX_LPRIM)


.PHONY: help
help:
	@echo "Default: validate-skip-summary-ml (generate small; use data/ for medium+large summaries)"
	@echo "Common:"
	@echo "  make validate            # full validate (generate all summaries)"
	@echo "  make validate-medium     # validate through medium"
	@echo "  make validate-large      # validate through large"
	@echo "  make validate-skip-summary-ml  # generate SGB all sizes, summary small only"
	@echo "  make clean-small         # clean small size files (1M, 17PR2)"
	@echo "  make clean-medium        # clean medium size files (10M, 19PR)"
	@echo "  make clean-large         # clean large size files (100M, 23PR)"
	@echo "  make clobber             # remove output artifacts; keep data/"
	@echo "  make clobber-small       # remove small data files (1M, 17PR2)"
	@echo "  make clobber-medium      # remove medium data files (10M, 19PR)"
	@echo "  make clobber-large       # remove large data files (100M, 23PR)"


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

test-touch: output/test-touch.stamp
output/test-touch.stamp:
	@echo "Creating test file"
	@touch "$@"
