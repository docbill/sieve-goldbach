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
LIMIT        := 500000000
GBCOUNT      := 10000
FORMATS_HL_A := full raw norm
FORMATS_EMP  := full raw norm cps
COMPAT       := v0.1.6
COMPAT_LEGACY := v0.1.5


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
START_LARGEF := 110000000
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
END_LARGE   := $(END_LARGEE)

SUFFIX_LPRIM  := 23PR.5
END_LPRIM   := $(END_LPRIMF)

SUFFIX_HUGE   := dummy

LARGEPARTS := A B C D E F
LPRIMPARTS := A B C D E F

# ==== 23#/2 -> 23# in single (19#/2) steps ====
# HUGE parts are dummy decade parts for XPRIM primorial parts
HUGEPARTS := A B C D E F G H I J K L M N O P Q R S T U V W

# Dummy HUGE parts with zero range (start = end) matching XPRIM ranges exactly
SUFFIX_HUGEA := dummyA
SUFFIX_HUGEB := dummyB
SUFFIX_HUGEC := dummyC
SUFFIX_HUGED := dummyD
SUFFIX_HUGEE := dummyE
SUFFIX_HUGEF := dummyF
SUFFIX_HUGEG := dummyG
SUFFIX_HUGEH := dummyH
SUFFIX_HUGEI := dummyI
SUFFIX_HUGEJ := dummyJ
SUFFIX_HUGEK := dummyK
SUFFIX_HUGEL := dummyL
SUFFIX_HUGEM := dummyM
SUFFIX_HUGEN := dummyN
SUFFIX_HUGEO := dummyO
SUFFIX_HUGEP := dummyP
SUFFIX_HUGEQ := dummyQ
SUFFIX_HUGER := dummyR
SUFFIX_HUGES := dummyS
SUFFIX_HUGET := dummyT
SUFFIX_HUGEU := dummyU
SUFFIX_HUGEV := dummyV
SUFFIX_HUGEW := dummyW

# Unit = 19#/2 = 4,849,845

SUFFIX_XPRIMA := 19PR23D2
START_XPRIMA := $(END_LPRIMF)
END_XPRIMA := 116396280

SUFFIX_XPRIMB := 19PR24D2
START_XPRIMB := $(END_XPRIMA)
END_XPRIMB := 121246125

SUFFIX_XPRIMC := 19PR25D2
START_XPRIMC := $(END_XPRIMB)
END_XPRIMC := 126095970

SUFFIX_XPRIMD := 19PR26D2
START_XPRIMD := $(END_XPRIMC)
END_XPRIMD := 130945815

SUFFIX_XPRIME := 19PR27D2
START_XPRIME := $(END_XPRIMD)
END_XPRIME := 135795660

SUFFIX_XPRIMF := 19PR28D2
START_XPRIMF := $(END_XPRIME)
END_XPRIMF := 140645505

SUFFIX_XPRIMG := 19PR29D2
START_XPRIMG := $(END_XPRIMF)
END_XPRIMG := 145495350

SUFFIX_XPRIMH := 19PR30D2
START_XPRIMH := $(END_XPRIMG)
END_XPRIMH := 150345195

SUFFIX_XPRIMI := 19PR31D2
START_XPRIMI := $(END_XPRIMH)
END_XPRIMI := 155195040

SUFFIX_XPRIMJ := 19PR32D2
START_XPRIMJ := $(END_XPRIMI)
END_XPRIMJ := 160044885

SUFFIX_XPRIMK := 19PR33D2
START_XPRIMK := $(END_XPRIMJ)
END_XPRIMK := 164894730

SUFFIX_XPRIML := 19PR34D2
START_XPRIML := $(END_XPRIMK)
END_XPRIML := 169744575

SUFFIX_XPRIMM := 19PR35D2
START_XPRIMM := $(END_XPRIML)
END_XPRIMM := 174594420

SUFFIX_XPRIMN := 19PR36D2
START_XPRIMN := $(END_XPRIMM)
END_XPRIMN := 179444265

SUFFIX_XPRIMO := 19PR37D2
START_XPRIMO := $(END_XPRIMN)
END_XPRIMO := 184294110

SUFFIX_XPRIMP := 19PR38D2
START_XPRIMP := $(END_XPRIMO)
END_XPRIMP := 189143955

SUFFIX_XPRIMQ := 19PR39D2
START_XPRIMQ := $(END_XPRIMP)
END_XPRIMQ := 193993800

SUFFIX_XPRIMR := 19PR40D2
START_XPRIMR := $(END_XPRIMQ)
END_XPRIMR := 198843645

SUFFIX_XPRIMS := 19PR41D2
START_XPRIMS := $(END_XPRIMR)
END_XPRIMS := 203693490

SUFFIX_XPRIMT := 19PR42D2
START_XPRIMT := $(END_XPRIMS)
END_XPRIMT := 208543335

SUFFIX_XPRIMU := 19PR43D2
START_XPRIMU := $(END_XPRIMT)
END_XPRIMU := 213393180

SUFFIX_XPRIMV := 19PR44D2
START_XPRIMV := $(END_XPRIMU)
END_XPRIMV := 218243025

SUFFIX_XPRIMW := 19PR45D2
START_XPRIMW := $(END_XPRIMV)
END_XPRIMW := 223092870  # = 23#

SUFFIX_XPRIM  := 23PR
END_XPRIM     := $(END_XPRIMW)

XPRIMPARTS := A B C D E F G H I J K L M N O P Q R S T U V W

# Size axes
SIZES         := SMALL SPRIM MEDIUM MPRIM LARGE LPRIM HUGE XPRIM

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
	@echo "Searching for partial files..."
	@TMPFILE=$$(mktemp); \
	find "$(OUT)" -name \*-$(COMPAT).partial.csv -type f ! -name "._*" > "$$TMPFILE"; \
	if [ -s "$$TMPFILE" ]; then \
		echo "Found $$(wc -l < "$$TMPFILE") partial files to backup"; \
		COPYFILE_DISABLE=1 tar cvvfj "$(BACKUP_FILE)" -T "$$TMPFILE" --exclude="._*" --exclude=".DS_Store" --no-xattrs 2>/dev/null || \
		COPYFILE_DISABLE=1 tar cvvfj "$(BACKUP_FILE)" -T "$$TMPFILE" --exclude="._*" --exclude=".DS_Store"; \
		if [ $$? -eq 0 ]; then \
			echo "Created \"$(BACKUP_FILE)\""; \
			tar -tjf "$(BACKUP_FILE)" >/dev/null 2>&1 && echo "Backup verified successfully" || echo "WARNING: Backup verification failed"; \
		else \
			echo "ERROR: Failed to create backup"; \
			rm -f "$$TMPFILE" "$(BACKUP_FILE)"; \
			exit 1; \
		fi; \
		rm -f "$$TMPFILE"; \
	else \
		echo "No partial files found to backup"; \
		touch "$(BACKUP_FILE)"; \
	fi
	test -z "$(LAST_BACKUP_FILE)" || if ( cmp "$(LAST_BACKUP_FILE)" "$(BACKUP_FILE)" ); then $(RM) "$(LAST_BACKUP_FILE)" ; fi

backup: $(BACKUP_FILE)

restore: 
	@test -n "$(LAST_BACKUP_FILE)" || (echo "$(BACKUP_TPL) not found." 1>&2 && exit 1)
	@tar xvvfj "$(LAST_BACKUP_FILE)"

# ---------- Base outputs (non-S/M/L) ----------
BITMAP_FILE := primes-500M.bitmap
RAW_FILE    := primes-500M.raw
GBP_FILE    := gbpairs-10000.csv

BITMAP := $(OUT)/$(BITMAP_FILE)
RAW    := $(OUT)/$(RAW_FILE)
GBP    := $(OUT)/$(GBP_FILE)

# Ensure output dir exists (order-only)
$(OUT):
	mkdir -p "$(OUT)"

# Generate base artifacts
$(BITMAP):: $(PRIME_BITMAP_BIN) $(OUT)
	./$(PRIME_BITMAP_BIN) $(LIMIT) > "$@"

$(RAW): $(BITMAP) | $(STOREPRIMES_BIN) $(OUT)
	./$(STOREPRIMES_BIN) "$(BITMAP)" "$@"

$(GBP): $(FINDGBPAIRS_BIN) | $(RAW) $(OUT)
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
	$$(LAVG_$(1)).csv $$(LMIN_$(1)).csv \
	$$(LMAX_$(1)).csv $$(LSAVG_$(1)).csv $$(LSMIN_$(1)).csv $$(LSMAX_$(1)).csv

# Verifies (sha256 or tool-specific)
SGB_VERIFY_$(1)      := $$(SGB_$(1)).csv.verify
SUMMARY_VERIFY_$(1)  := $$(SUMMARY_$(1)).csv.verify
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
# Conditionally handle dummy parts (HUGE has no START defined) vs real parts
ifeq ($(1),HUGE)
# Dummy part (HUGE) - mark summary, sgb, lambda, join, cps, and verification rules as phony
.PHONY: $$(SUMMARY_$(1)).csv $$(SGB_$(1)).csv \
        $$(SUMMARY_$(1)).csv.verify $$(SGB_$(1)).csv.verify \
        $$(JOIN_DEFAULT_$(1)).csv $$(JOIN_$(1)).csv $$(JOIN_VERIFY_$(1)) \
        $$(CPSLB_$(1)).csv $$(CPSLB_VERIFY_$(1)) \
        $$(CPS_SUMMARY_VERIFY_$(1)) \
        $$(LAVG_DEFAULT_$(1)).csv $$(LAVG_$(1)).csv $$(LAVG_VERIFY_$(1)) \
        $$(LMIN_DEFAULT_$(1)).csv $$(LMIN_$(1)).csv $$(LMIN_VERIFY_$(1)) \
        $$(LMAX_DEFAULT_$(1)).csv $$(LMAX_$(1)).csv $$(LMAX_VERIFY_$(1)) \
        $$(LSAVG_DEFAULT_$(1)).csv $$(LSAVG_$(1)).csv $$(LSAVG_VERIFY_$(1)) \
        $$(LSMIN_DEFAULT_$(1)).csv $$(LSMIN_$(1)).csv $$(LSMIN_VERIFY_$(1))
else
# Real part - generate rules normally

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
	@set -Eeo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		summary_src="$$(call GET,SUMMARY_TPL,$(1))"; sgb_src="$$(call GET,SGB_TPL,$(1))"; \
		summary_src="$$$${summary_src//-=ALPHA=-/$$$$a}"; sgb_src="$$$${sgb_src//-=ALPHA=-/$$$$a}"; \
		summary_src="$$$${summary_src//-=FORMAT=-/full}"; sgb_src="$$$${sgb_src//-=FORMAT=-/full}"; \
		if [ -r "$$$$summary_src.csv" ] && [ -r "$$$$sgb_src.csv" ]; then \
			dst="$$(call GET,JOIN_TPL,$(1))"; \
			dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
			export VERSION=$(COMPAT); ./bin/joinSumPred.awk -v alpha=$$$$a "$$$$summary_src.csv" "$$$$sgb_src.csv" > "$$$$dst.csv"; \
		fi; \
	done

$$(JOIN_$(1)).csv: $$(JOIN_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

# CPS lower bound (needs n_0 from SUMMARY)
$$(CPSLB_$(1)).csv: $(CPSLB_BIN) $$(SUMMARY_$(1)).csv $(SUMMARY_DEFAULT_$(1)).csv | $(RAW)
	cps_src="$$(call GET,CPSLB_DEFAULT,$(1)).csv"; \
	if [ -r "$$$$cps_src" ]; then \
		cp "$$$$cps_src" "$$@"; \
	else \
		echo "Skipping CPS copy - file does not exist (dummy part)" ; \
		touch "$$@"; \
	fi

$$(LAVG_DEFAULT_$(1)).csv: $$(SUMMARY_DEFAULT_$(1)).csv $$(SGB_DEFAULT_$(1)).csv
	@chmod ugo+x ./bin/compareAvg.awk
	# Generate lambda files for all alphas
	@set -Eeo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		summary_src="$$(call GET,SUMMARY_TPL,$(1))"; sgb_src="$$(call GET,SGB_TPL,$(1))"; \
		summary_src="$$$${summary_src//-=ALPHA=-/$$$$a}"; sgb_src="$$$${sgb_src//-=ALPHA=-/$$$$a}"; \
		summary_src="$$$${summary_src//-=FORMAT=-/full}"; sgb_src="$$$${sgb_src//-=FORMAT=-/full}"; \
		if [ -r "$$$$summary_src.csv" ] && [ -r "$$$$sgb_src.csv" ]; then \
			lavg_dst="$$(call GET,LAVG_TPL,$(1))"; \
			lavg_dst="$$$${lavg_dst//-=ALPHA=-/$$$$a}"; \
			export VERSION=$(COMPAT); ./bin/compareAvg.awk "$$$$summary_src.csv" "$$$$sgb_src.csv" > "$$$$lavg_dst.csv"; \
		fi; \
	done

# Lambda CSVs - copy from alpha-0.5 versions
$$(LAVG_$(1)).csv: $$(LAVG_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

$$(LMIN_DEFAULT_$(1)).csv: $$(SUMMARY_DEFAULT_$(1)).csv $$(SGB_DEFAULT_$(1)).csv
	@chmod ugo+x ./bin/compareMin.awk
	# Generate lambda files for all alphas
	@set -Eeo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		summary_src="$$(call GET,SUMMARY_TPL,$(1))"; sgb_src="$$(call GET,SGB_TPL,$(1))"; \
		summary_src="$$$${summary_src//-=ALPHA=-/$$$$a}"; sgb_src="$$$${sgb_src//-=ALPHA=-/$$$$a}"; \
		summary_src="$$$${summary_src//-=FORMAT=-/full}"; sgb_src="$$$${sgb_src//-=FORMAT=-/full}"; \
		if [ -r "$$$$summary_src.csv" ] && [ -r "$$$$sgb_src.csv" ]; then \
			lmin_dst="$$(call GET,LMIN_TPL,$(1))"; \
			lmin_dst="$$$${lmin_dst//-=ALPHA=-/$$$$a}"; \
			export VERSION=$(COMPAT); ./bin/compareMin.awk "$$$$summary_src.csv" "$$$$sgb_src.csv" > "$$$$lmin_dst.csv"; \
		fi; \
	done

$$(LMIN_$(1)).csv: $$(LMIN_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

$$(LMAX_DEFAULT_$(1)).csv: $$(SUMMARY_DEFAULT_$(1)).csv $$(SGB_DEFAULT_$(1)).csv
	@chmod ugo+x ./bin/compareMax.awk
	# Generate lambda files for all alphas
	@set -Eeo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		summary_src="$$(call GET,SUMMARY_TPL,$(1))"; sgb_src="$$(call GET,SGB_TPL,$(1))"; \
		summary_src="$$$${summary_src//-=ALPHA=-/$$$$a}"; sgb_src="$$$${sgb_src//-=ALPHA=-/$$$$a}"; \
		summary_src="$$$${summary_src//-=FORMAT=-/full}"; sgb_src="$$$${sgb_src//-=FORMAT=-/full}"; \
		if [ -r "$$$$summary_src.csv" ] && [ -r "$$$$sgb_src.csv" ]; then \
			lmax_dst="$$(call GET,LMAX_TPL,$(1))"; \
			lmax_dst="$$$${lmax_dst//-=ALPHA=-/$$$$a}"; \
			export VERSION=$(COMPAT); ./bin/compareMax.awk "$$$$summary_src.csv" "$$$$sgb_src.csv" > "$$$$lmax_dst.csv"; \
		fi; \
	done

$$(LMAX_$(1)).csv: $$(LMAX_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

$$(LSAVG_DEFAULT_$(1)).csv: $$(LAVG_DEFAULT_$(1)).csv
	@chmod ugo+x ./bin/lambdaStats.awk; \
	# Generate lambda files for all alphas
	@set -Eeo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		lavg_dst="$$(call GET,LAVG_TPL,$(1))"; \
		lavg_dst="$$$${lavg_dst//-=ALPHA=-/$$$$a}"; \
		if [ -r "$$$$lavg_dst.csv" ]; then \
			./bin/lambdaStats.awk "$$$$lavg_dst.csv" > "$$$${lavg_dst/lambdaavg/lambdastatsavg}.csv"; \
		fi; \
	done

# Lambda Stats CSVs - copy from alpha-0.5 versions
$$(LSAVG_$(1)).csv: $$(LSAVG_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

$$(LSMIN_DEFAULT_$(1)).csv: $$(LMIN_DEFAULT_$(1)).csv
	@chmod ugo+x ./bin/lambdaStats.awk; \
	# Generate lambda files for all alphas
	@set -Eeo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		lmin_dst="$$(call GET,LMIN_TPL,$(1))"; \
		lmin_dst="$$$${lmin_dst//-=ALPHA=-/$$$$a}"; \
		if [ -r "$$$$lmin_dst.csv" ]; then \
			./bin/lambdaStats.awk "$$$$lmin_dst.csv" > "$$$${lmin_dst/lambdamin/lambdastatsmin}.csv"; \
		fi; \
	done

$$(LSMIN_$(1)).csv: $$(LSMIN_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

$$(LSMAX_DEFAULT_$(1)).csv: $$(LMAX_DEFAULT_$(1)).csv
	@chmod ugo+x ./bin/lambdaStats.awk; \
	# Generate lambda files for all alphas
	@set -Eeo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do \
		lmax_dst="$$(call GET,LMAX_TPL,$(1))"; \
		lmax_dst="$$$${lmax_dst//-=ALPHA=-/$$$$a}"; \
		if [ -r "$$$$lmax_dst.csv" ]; then \
			./bin/lambdaStats.awk "$$$$lmax_dst.csv" > "$$$${lmax_dst/lambdamax/lambdastatsmax}.csv"; \
		fi; \
	done

$$(LSMAX_$(1)).csv: $$(LSMAX_DEFAULT_$(1)).csv
	cp "$$<" "$$@"
	@touch "$$@"

endif
# End conditional: dummy parts (phony) vs real parts (generate rules)

# --- Verifications ---

# SUMMARY verify uses validator + SHA
$$(SUMMARY_$(1)).csv.verify: $$(SUMMARY_$(1)).csv | $(VALIDATESUMMARY) $(OUT) $(BITMAP) $(RAW)
	@set -Eeo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
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

$$(SGB_$(1)).csv.verify: $$(SGB_$(1)).csv | $(VALIDATESUMMARY) $(OUT) $(RAW) $(BITMAP)
	@set -Eeo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
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
	@for a in $(ALPHAS); do for fmt in $(FORMATS_EMP); do for dst in $(call GET,SUMMARY_TPL,$(1)) $(call GET,SGB_TPL,$(1)); do \
		dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
		dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
		$(RM) "$$$$dst.csv"{,.verify,.sha256}; \
	done; done; done
	@for a in $(ALPHAS); do for dst in $(call GET,LAVG_TPL,$(1)) $(call GET,LMIN_TPL,$(1)) $(call GET,LMAX_TPL,$(1)) \
	 $(call GET,LSAVG_TPL,$(1)) $(call GET,LSMIN_TPL,$(1)) $(call GET,LSMAX_TPL,$(1)) $(call GET,CPSLB_TPL,$(1)) $(call GET,JOIN_TPL,$(1)) ; do \
		$(RM) "$$$${dst//-=ALPHA=-/$$$$a}".csv{,.sha256,.stamp}; \
	done; done
	@$(RM) "$(call GET,JOIN,$(1))".csv{,.sha256}

clobber-$$(SFX_$(1)): clean-$$(SFX_$(1))
	for a in $(ALPHAS); do for fmt in $(FORMATS_EMP); do for dst in $(call GET,CPS_SUMMARY,$(1)) $(call GET,SUMMARY_TPL,$(1)) $(call GET,SGB_TPL,$(1)); do \
		dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
		dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
		sources=($(2)); \
		for suffix in $$(call SFX,$(1)A) $$(call SFX,$(1)B) $$(call SFX,$(1)C) $$(call SFX,$(1)D) $$(call SFX,$(1)E) $$(call SFX,$(1)F) \
		$(call SFX,$(1)G) $(call SFX,$(1)H) $(call SFX,$(1)I) $(call SFX,$(1)J) $(call SFX,$(1)K) $(call SFX,$(1)L) $(call SFX,$(1)M) \
		$(call SFX,$(1)N) $(call SFX,$(1)O) $(call SFX,$(1)P) $(call SFX,$(1)Q) $(call SFX,$(1)R) $(call SFX,$(1)S) $(call SFX,$(1)T) \
		$(call SFX,$(1)U) $(call SFX,$(1)V) $(call SFX,$(1)W) ; do \
			if [ -n "$$$$suffix" ]; then \
				sources+=("$$$${dst/-$$(call SFX,$(1))-/-$$$$suffix-}.partial.csv") ; \
			fi; \
		done; \
		echo $(RM) "$$$${sources[@]}" "$$$$dst.csv"; \
		$(RM) "$$$${sources[@]}" "$$$$dst.csv"; \
	done; done; done

touch-$$(SFX_$(1)): 
	for a in $(ALPHAS); do for fmt in $(FORMATS_EMP); do for dst in $(call GET,CPS_SUMMARY_TPL,$(1)) $(call GET,SUMMARY_TPL,$(1)) $(call GET,SGB_TPL,$(1)); do \
		dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
		dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
		sources=($(2)); \
		for suffix in $$(call SFX,$(1)A) $$(call SFX,$(1)B) $$(call SFX,$(1)C) $$(call SFX,$(1)D) $$(call SFX,$(1)E) $$(call SFX,$(1)F) \
		$(call SFX,$(1)G) $(call SFX,$(1)H) $(call SFX,$(1)I) $(call SFX,$(1)J) $(call SFX,$(1)K) $(call SFX,$(1)L) $(call SFX,$(1)M) \
		$(call SFX,$(1)N) $(call SFX,$(1)O) $(call SFX,$(1)P) $(call SFX,$(1)Q) $(call SFX,$(1)R) $(call SFX,$(1)S) $(call SFX,$(1)T) \
		$(call SFX,$(1)U) $(call SFX,$(1)V) $(call SFX,$(1)W) ; do \
			if [ -n "$$$$suffix" ]; then \
				sources+=("$$$${dst/-$$(call SFX,$(1))-/-$$$$suffix-}.partial.csv") ; \
			fi; \
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

$$(call GET,SUMMARY_DEFAULT,$(1)).partial.csv: $(SUMMARY_BIN) | $(RAW) $(OUT) 
	@set -Eeo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do mkdir -p "$(OUT)/alpha-$$$$a"; done ; \
	trace=(); \
	dec=(); \
	out_flag="full"; \
	if [ "$(COMPAT)" = "v0.1.5" ]; then \
		out_flag="out"; \
	fi; \
	if [ -n "$$(call GET,SUMMARY_TPL,$(1))" ] && [ -n "$$(call GET,START,$(1))" ] && [ -n "$$(call GET,END,$(1))" ]; then \
		trace=( --trace=decade ); \
		dec=( --dec-n-start $$(call GET,START,$(1)) --dec-n-end $$(call GET,END,$(1)) \
			--dec-"$$$$out_flag"="$$(call GET,SUMMARY_TPL,$(1)).partial.csv" \
			--dec-cps="$$(call GET,SUMMARY_TPL,$(1)).partial.csv" \
			--dec-cps-summary="$$(call GET,CPS_SUMMARY,$(1)).partial.csv" ); \
	fi ; \
	prim=(); \
	if [ -n "$$(call GET,SUMMARY_TPL,$(2))" ] && [ -n "$$(call GET,START,$(2))" ] && [ -n "$$(call GET,END,$(2))" ]; then \
		trace=( --trace=primorial ); \
		prim=( --prim-n-start $$(call GET,START,$(2)) --prim-n-end $$(call GET,END,$(2)) \
			--prim-"$$$$out_flag"="$$(call GET,SUMMARY_TPL,$(2)).partial.csv" \
			--prim-cps="$$(call GET,SUMMARY_TPL,$(2)).partial.csv" \
			--prim-cps-summary="$$(call GET,CPS_SUMMARY,$(2)).partial.csv" ); \
	fi ; \
	if [ $$$${#dec[@]} -gt 0 ] || [ $$$${#prim[@]} -gt 0 ]; then \
		echo ./$(SUMMARY_BIN) $(ALPHA_ARGS) --compat=$(COMPAT) --model=empirical \
			"$$$${dec[@]}" "$$$${prim[@]}" "$$$${trace[@]}" "$(RAW)" ; \
		./$(SUMMARY_BIN) $(ALPHA_ARGS) --compat=$(COMPAT) --model=empirical \
			"$$$${dec[@]}" "$$$${prim[@]}" "$$$${trace[@]}" "$(RAW)" ; \
	else \
		echo "Skipping gbpairsummary call - no valid parameters (dummy part)" ; \
	fi
	touch "$$@"

$$(call GET,SUMMARY_DEFAULT,$(2)).partial.csv: $$(call GET,SUMMARY_DEFAULT,$(1)).partial.csv
	@true

$$(call GET,CPS_SUMMARY,$(1)).partial.csv: $$(call GET,SUMMARY_DEFAULT,$(1)).partial.csv
	@true

$$(call GET,CPS_SUMMARY,$(2)).partial.csv: $$(call GET,SUMMARY_DEFAULT,$(2)).partial.csv
	@true

$$(call GET,SGB_DEFAULT,$(1)).partial.csv: $(SUMMARY_BIN) | $(RAW) $(OUT)
	@set -Eeo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do mkdir -p "$(OUT)/alpha-$$$$a"; done ; \
	trace=(); \
	dec=(); \
	prim=(); \
	out_flag="full"; \
	if [ "$(COMPAT)" = "v0.1.5" ]; then \
		out_flag="out"; \
	fi; \
	if [ -n "$$(call GET,SGB_TPL,$(1))" ] && [ -n "$$(call GET,START,$(1))" ] && [ -n "$$(call GET,END,$(1))" ]; then \
		trace=( --trace=decade ); \
		dec=( --dec-n-start $$(call GET,START,$(1)) --dec-n-end $$(call GET,END,$(1)) \
			--dec-"$$$$out_flag"="$$(call GET,SGB_TPL,$(1)).partial.csv" ); \
	fi ; \
	if [ -n "$$(call GET,SGB_TPL,$(2))" ] && [ -n "$$(call GET,START,$(2))" ] && [ -n "$$(call GET,END,$(2))" ]; then \
		trace=( --trace=primorial ); \
		prim=( --prim-n-start $$(call GET,START,$(2)) --prim-n-end $$(call GET,END,$(2)) \
			--prim-"$$$$out_flag"="$$(call GET,SGB_TPL,$(2)).partial.csv" ); \
	fi ; \
	if [ $$$${#dec[@]} -gt 0 ] || [ $$$${#prim[@]} -gt 0 ]; then \
		echo ./$(SUMMARY_BIN) $(ALPHA_ARGS) --compat=$(COMPAT) --model=hl-a \
			"$$$${dec[@]}" "$$$${prim[@]}" "$$$${trace[@]}" "$(RAW)" ; \
		./$(SUMMARY_BIN) $(ALPHA_ARGS) --compat=$(COMPAT) --model=hl-a \
			"$$$${dec[@]}" "$$$${prim[@]}" "$$$${trace[@]}" "$(RAW)" ; \
	else \
		echo "Skipping gbpairsummary call - no valid parameters (dummy part)" ; \
	fi
	touch "$$@"


$$(call GET,SGB_DEFAULT,$(2)).partial.csv: $(call GET,SGB_DEFAULT,$(1)).partial.csv
	@true

endef

# SMALL

define PARTS_TEMPLATE2


$$(SUMMARY_DEFAULT_$(1)).csv: \
        $(foreach part,$($(1)PARTS),$$(SUMMARY_DEFAULT_$(1)$(part)).partial.csv) \
        $(call IFSIZE,$(2),$(foreach part,$($(2)PARTS),$$(SUMMARY_DEFAULT_$(2)$(part)).partial.csv)) \
        $(call IFSIZE,$(3),$(foreach part,$($(3)PARTS),$$(SUMMARY_DEFAULT_$(3)$(part)).partial.csv)) 
	@echo "Merging: $$@"
	@chmod ugo+x ./bin/mergeCPSLowerBound.awk
	@set -Eeo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do for fmt in $(FORMATS_EMP); do \
		sources=(); \
		prior_merged=(); \
		if [ "$(1)" = "XPRIM" ]; then \
			prior_dst="$$(call GET,SUMMARY_TPL,LPRIM)"; \
			prior_dst="$$$${prior_dst//-=ALPHA=-/$$$$a}"; \
			prior_dst="$$$${prior_dst//-=FORMAT=-/$$$$fmt}"; \
			if [ -r "$$$$prior_dst.csv" ]; then \
				prior_merged+=("$$$$prior_dst.csv"); \
			fi; \
		fi; \
		if [ -n "$(3)" ]; then \
			dst="$$(call GET,SUMMARY_TPL,$(3))"; \
			dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
			dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
			for suffix in $$(call SFX,$(3)A) $$(call SFX,$(3)B) $$(call SFX,$(3)C) $$(call SFX,$(3)D) $$(call SFX,$(3)E) $$(call SFX,$(3)F) \
			$(call SFX,$(3)G) $(call SFX,$(3)H) $(call SFX,$(3)I) $(call SFX,$(3)J) $(call SFX,$(3)K) $(call SFX,$(3)L) $(call SFX,$(3)M) \
			$(call SFX,$(3)N) $(call SFX,$(3)O) $(call SFX,$(3)P) $(call SFX,$(3)Q) $(call SFX,$(3)R) $(call SFX,$(3)S) $(call SFX,$(3)T) \
			$(call SFX,$(3)U) $(call SFX,$(3)V) $(call SFX,$(3)W) ; do \
				file="$$$${dst/-$$(call SFX,$(3))-/-$$$$suffix-}.partial.csv"; \
				if [ -r "$$$$file" ]; then \
					sources+=("$$$$file"); \
				fi; \
			done; \
		fi; \
		if [ -n "$(2)" ]; then \
			dst="$$(call GET,SUMMARY_TPL,$(2))"; \
			dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
			dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
			for suffix in $$(call SFX,$(2)A) $$(call SFX,$(2)B) $$(call SFX,$(2)C) $$(call SFX,$(2)D) $$(call SFX,$(2)E) $$(call SFX,$(2)F) \
			$(call SFX,$(2)G) $(call SFX,$(2)H) $(call SFX,$(2)I) $(call SFX,$(2)J) $(call SFX,$(2)K) $(call SFX,$(2)L) $(call SFX,$(2)M) \
			$(call SFX,$(2)N) $(call SFX,$(2)O) $(call SFX,$(2)P) $(call SFX,$(2)Q) $(call SFX,$(2)R) $(call SFX,$(2)S) $(call SFX,$(2)T) \
			$(call SFX,$(2)U) $(call SFX,$(2)V) $(call SFX,$(2)W) ; do \
				file="$$$${dst/-$$(call SFX,$(2))-/-$$$$suffix-}.partial.csv"; \
				if [ -r "$$$$file" ]; then \
					sources+=("$$$$file"); \
				fi; \
			done; \
		fi; \
		dst="$$(call GET,SUMMARY_TPL,$(1))"; \
		dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
		dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
		for suffix in $$(call SFX,$(1)A) $$(call SFX,$(1)B) $$(call SFX,$(1)C) $$(call SFX,$(1)D) $$(call SFX,$(1)E) $$(call SFX,$(1)F) \
		$(call SFX,$(1)G) $(call SFX,$(1)H) $(call SFX,$(1)I) $(call SFX,$(1)J) $(call SFX,$(1)K) $(call SFX,$(1)L) $(call SFX,$(1)M) \
		$(call SFX,$(1)N) $(call SFX,$(1)O) $(call SFX,$(1)P) $(call SFX,$(1)Q) $(call SFX,$(1)R) $(call SFX,$(1)S) $(call SFX,$(1)T) \
		$(call SFX,$(1)U) $(call SFX,$(1)V) $(call SFX,$(1)W) ; do \
			file="$$$${dst/-$$(call SFX,$(1))-/-$$$$suffix-}.partial.csv"; \
			if [ -r "$$$$file" ]; then \
				sources+=("$$$$file"); \
			fi; \
		done; \
		if [ -n "$$$${prior_merged[*]}" ]; then \
			sources=("$$$${prior_merged[@]}" "$$$${sources[@]}"); \
		fi; \
		if [ -n "$$$${sources[*]}" ]; then  \
			if [ "cps" = "$$$$fmt" ]; then \
				./bin/mergeCPSLowerBound.awk "$$$${sources[@]}" > "$$$$dst.csv" ; \
			else \
				(head -1 "$$$${sources[0]}";for s in "$$$${sources[@]}"; do \
					tail -n +2 "$$$$s"; \
			done) > "$$$$dst.csv"; \
			if [[ "$(COMPAT)" != v0.1.* ]] && [ "full" = "$$$$fmt" ]; then \
				./bin/full2norm_empirical.awk "$$$$dst.csv" > "$$$${dst/-full-/-norm-}.csv"; \
					./bin/full2raw_empirical.awk "$$$$dst.csv" > "$$$${dst/-full-/-raw-}.csv"; \
				fi; \
			fi; \
		fi; \
	done; done
	@touch "$$@"

$$(SGB_DEFAULT_$(1)).csv: \
        $(foreach part,$($(1)PARTS),$$(SGB_DEFAULT_$(1)$(part)).partial.csv) \
        $(call IFSIZE,$(2),$(foreach part,$($(2)PARTS),$$(SGB_DEFAULT_$(2)$(part)).partial.csv)) \
        $(call IFSIZE,$(3),$(foreach part,$($(3)PARTS),$$(SGB_DEFAULT_$(3)$(part)).partial.csv)) 
	@echo "Merging: $$@"
	@set -Eeo pipefail; trap 'echo "error at line $$$$LINENO" >&2; exit 1' ERR; \
	for a in $(ALPHAS); do for fmt in $(FORMATS_EMP); do \
		sources=(); \
		prior_merged=(); \
		if [ "$(1)" = "XPRIM" ]; then \
			prior_dst="$$(call GET,SGB_TPL,LPRIM)"; \
			prior_dst="$$$${prior_dst//-=ALPHA=-/$$$$a}"; \
			prior_dst="$$$${prior_dst//-=FORMAT=-/$$$$fmt}"; \
			if [ -r "$$$$prior_dst.csv" ]; then \
				prior_merged+=("$$$$prior_dst.csv"); \
			fi; \
		fi; \
		if [ -n "$(3)" ]; then \
			dst="$$(call GET,SGB_TPL,$(3))"; \
			dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
			dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
			for suffix in $$(call SFX,$(3)A) $$(call SFX,$(3)B) $$(call SFX,$(3)C) $$(call SFX,$(3)D) $$(call SFX,$(3)E) $$(call SFX,$(3)F) \
			$$(call SFX,$(3)G) $$(call SFX,$(3)H) $$(call SFX,$(3)I) $$(call SFX,$(3)J) $$(call SFX,$(3)K) $$(call SFX,$(3)L) $$(call SFX,$(3)M) \
			$$(call SFX,$(3)N) $$(call SFX,$(3)O) $$(call SFX,$(3)P) $$(call SFX,$(3)Q) $$(call SFX,$(3)R) $$(call SFX,$(3)S) $$(call SFX,$(3)T) \
			$$(call SFX,$(3)U) $$(call SFX,$(3)V) $$(call SFX,$(3)W) : ; do \
				if [ -n "$$$$suffix" ] && [ "$$$$suffix" != ":" ]; then \
					file="$$$${dst/-$$(call SFX,$(3))-/-$$$$suffix-}.partial.csv"; \
					if [ -r "$$$$file" ]; then \
						sources+=("$$$$file"); \
					fi; \
				fi; \
			done; \
		fi; \
		if [ -n "$(2)" ]; then \
			dst="$$(call GET,SGB_TPL,$(2))"; \
			dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
			dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
			for suffix in $$(call SFX,$(2)A) $$(call SFX,$(2)B) $$(call SFX,$(2)C) $$(call SFX,$(2)D) $$(call SFX,$(2)E) $$(call SFX,$(2)F) \
			$$(call SFX,$(2)G) $$(call SFX,$(2)H) $$(call SFX,$(2)I) $$(call SFX,$(2)J) $$(call SFX,$(2)K) $$(call SFX,$(2)L) $$(call SFX,$(2)M) \
			$$(call SFX,$(2)N) $$(call SFX,$(2)O) $$(call SFX,$(2)P) $$(call SFX,$(2)Q) $$(call SFX,$(2)R) $$(call SFX,$(2)S) $$(call SFX,$(2)T) \
			$$(call SFX,$(2)U) $$(call SFX,$(2)V) $$(call SFX,$(2)W) "" ; do \
				if [ -n "$$$$suffix" ] && [ "$$$$suffix" != ":" ]; then \
					file="$$$${dst/-$$(call SFX,$(2))-/-$$$$suffix-}.partial.csv"; \
					if [ -r "$$$$file" ]; then \
						sources+=("$$$$file"); \
					fi; \
				fi; \
			done; \
		fi; \
		dst="$$(call GET,SGB_TPL,$(1))"; \
		dst="$$$${dst//-=ALPHA=-/$$$$a}"; \
		dst="$$$${dst//-=FORMAT=-/$$$$fmt}"; \
		for suffix in $$(call SFX,$(1)A) $$(call SFX,$(1)B) $$(call SFX,$(1)C) $$(call SFX,$(1)D) $$(call SFX,$(1)E) $$(call SFX,$(1)F) \
		$(call SFX,$(1)G) $(call SFX,$(1)H) $(call SFX,$(1)I) $(call SFX,$(1)J) $(call SFX,$(1)K) $(call SFX,$(1)L) $(call SFX,$(1)M) \
		$(call SFX,$(1)N) $(call SFX,$(1)O) $(call SFX,$(1)P) $(call SFX,$(1)Q) $(call SFX,$(1)R) $(call SFX,$(1)S) $(call SFX,$(1)T) \
		$(call SFX,$(1)U) $(call SFX,$(1)V) $(call SFX,$(1)W) "" ; do \
			if [ -n "$$$$suffix" ] && [ "$$$$suffix" != "" ]; then \
				file="$$$${dst/-$$(call SFX,$(1))-/-$$$$suffix-}.partial.csv"; \
				if [ -r "$$$$file" ]; then \
					sources+=("$$$$file"); \
				fi; \
			fi; \
		done; \
		if [ -n "$$$${prior_merged[*]}" ]; then \
			sources=("$$$${prior_merged[@]}" "$$$${sources[@]}"); \
		fi; \
		if [ -n "$$$${sources[*]}" ] && [ "cps" != "$$$$fmt" ] ; then  \
			(head -1 "$$$${sources[0]}";for s in "$$$${sources[@]}"; do \
				tail -n +2 "$$$$s"; \
			done) > "$$$$dst.csv"; \
			if [[ "$(COMPAT)" != v0.1.* ]] && [ "full" = "$$$$fmt" ]; then \
				./bin/full2norm_hla.awk "$$$$dst.csv" > "$$$${dst/-full-/-norm-}.csv"; \
				./bin/full2raw_hla.awk "$$$$dst.csv" > "$$$${dst/-full-/-raw-}.csv"; \
			fi; \
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
	@set -Eeo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	$(MERGECPS) $(foreach a,$(CPS_SUMMARY_SMALL_PARTS),--input $(a)) --output "$@"

$(eval $(call PARTS_TEMPLATE2,SPRIM,,))

$(CPS_SUMMARY_SPRIM).csv: $(SUMMARY_DEFAULT_SPRIM).csv | $(MERGECPS)
	@set -Eeo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	$(MERGECPS) $(foreach a,$(CPS_SUMMARY_SPRIM_PARTS),--input $(a)) --output "$@"

# MEDIUM
$(foreach PRT,$(MEDIUMPARTS),$(eval $(call SUMMARY_TEMPLATE,MEDIUM$(PRT))))

CPS_SUMMARY_MEDIUM_PARTS := $(CPS_SUMMARY_MEDIUMA).partial.csv $(CPS_SUMMARY_MEDIUMB).partial.csv $(CPS_SUMMARY_MEDIUMC).partial.csv $(CPS_SUMMARY_MEDIUMD).partial.csv $(CPS_SUMMARY_MEDIUME).partial.csv

$(foreach PRT,$(MEDIUMPARTS),$(eval $(call SUMMARY_TEMPLATE,MPRIM$(PRT))))

CPS_SUMMARY_MPRIM_PARTS := $(CPS_SUMMARY_MPRIMA).partial.csv $(CPS_SUMMARY_MPRIMB).partial.csv $(CPS_SUMMARY_MPRIMC).partial.csv $(CPS_SUMMARY_MPRIMD).partial.csv $(CPS_SUMMARY_MPRIME).partial.csv

$(foreach PRT,$(MEDIUMPARTS),$(eval $(call PARTS_TEMPLATE,MEDIUM$(PRT),MPRIM$(PRT),MEDIUM,MPRIM)))

$(eval $(call PARTS_TEMPLATE2,MEDIUM,SMALL,))

$(CPS_SUMMARY_MEDIUM).csv: $(CPS_SUMMARY_SMALL).csv $(SUMMARY_DEFAULT_MEDIUM).csv | $(MERGECPS)
	@set -Eeo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	$(MERGECPS) $(foreach a,$(CPS_SUMMARY_SMALL).csv $(CPS_SUMMARY_MEDIUM_PARTS),--input $(a)) --output "$@"

$(eval $(call PARTS_TEMPLATE2,MPRIM,SPRIM,))

$(CPS_SUMMARY_MPRIM).csv: $(CPS_SUMMARY_SPRIM).csv $(SUMMARY_DEFAULT_MPRIM).csv | $(MERGECPS)
	@set -Eeo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	$(MERGECPS) $(foreach a,$(CPS_SUMMARY_SPRIM).csv $(CPS_SUMMARY_MPRIM_PARTS),--input $(a)) --output "$@"

# LARGE

$(foreach PRT,$(LARGEPARTS),$(eval $(call SUMMARY_TEMPLATE,LARGE$(PRT))))

CPS_SUMMARY_LARGE_PARTS := $(CPS_SUMMARY_LARGEA).partial.csv $(CPS_SUMMARY_LARGEB).partial.csv $(CPS_SUMMARY_LARGEC).partial.csv $(CPS_SUMMARY_LARGED).partial.csv $(CPS_SUMMARY_LARGEE).partial.csv $(CPS_SUMMARY_LARGEF).partial.csv

$(foreach PRT,$(LARGEPARTS),$(eval $(call SUMMARY_TEMPLATE,LPRIM$(PRT))))

CPS_SUMMARY_LPRIM_PARTS := $(CPS_SUMMARY_LPRIMA).partial.csv $(CPS_SUMMARY_LPRIMB).partial.csv $(CPS_SUMMARY_LPRIMC).partial.csv $(CPS_SUMMARY_LPRIMD).partial.csv $(CPS_SUMMARY_LPRIME).partial.csv $(CPS_SUMMARY_LPRIMF).partial.csv

# HUGE (dummy decade parts for XPRIM)
$(foreach PRT,$(HUGEPARTS),$(eval $(call SUMMARY_TEMPLATE,HUGE$(PRT))))

# XPRIM (primorial parts)
$(foreach PRT,$(XPRIMPARTS),$(eval $(call SUMMARY_TEMPLATE,XPRIM$(PRT))))

CPS_SUMMARY_XPRIM_PARTS := $(CPS_SUMMARY_XPRIMA).partial.csv $(CPS_SUMMARY_XPRIMB).partial.csv $(CPS_SUMMARY_XPRIMC).partial.csv $(CPS_SUMMARY_XPRIMD).partial.csv $(CPS_SUMMARY_XPRIME).partial.csv $(CPS_SUMMARY_XPRIMF).partial.csv \
	$(CPS_SUMMARY_XPRIMG).partial.csv $(CPS_SUMMARY_XPRIMH).partial.csv $(CPS_SUMMARY_XPRIMI).partial.csv $(CPS_SUMMARY_XPRIMJ).partial.csv $(CPS_SUMMARY_XPRIMK).partial.csv $(CPS_SUMMARY_XPRIML).partial.csv $(CPS_SUMMARY_XPRIMM).partial.csv \
	$(CPS_SUMMARY_XPRIMN).partial.csv $(CPS_SUMMARY_XPRIMO).partial.csv $(CPS_SUMMARY_XPRIMP).partial.csv $(CPS_SUMMARY_XPRIMQ).partial.csv $(CPS_SUMMARY_XPRIMR).partial.csv $(CPS_SUMMARY_XPRIMS).partial.csv $(CPS_SUMMARY_XPRIMT).partial.csv \
	$(CPS_SUMMARY_XPRIMU).partial.csv $(CPS_SUMMARY_XPRIMV).partial.csv $(CPS_SUMMARY_XPRIMW).partial.csv

$(foreach PRT,$(LARGEPARTS),$(eval $(call PARTS_TEMPLATE,LARGE$(PRT),LPRIM$(PRT),LARGE,LPRIM)))

# HUGE/XPRIM pairing (dummy decade + real primorial)
$(foreach PRT,$(HUGEPARTS),$(eval $(call PARTS_TEMPLATE,HUGE$(PRT),XPRIM$(PRT),HUGE,XPRIM)))

$(eval $(call PARTS_TEMPLATE2,LARGE,MEDIUM,SMALL))

$(CPS_SUMMARY_LARGE).csv: $(CPS_SUMMARY_MEDIUM).csv $(SUMMARY_DEFAULT_LARGE).csv | $(MERGECPS)
	@set -Eeo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	$(MERGECPS) $(foreach a,$(CPS_SUMMARY_MEDIUM).csv $(CPS_SUMMARY_LARGE_PARTS),--input $(a)) --output "$@"

$(eval $(call PARTS_TEMPLATE2,LPRIM,MPRIM,SPRIM))

$(eval $(call PARTS_TEMPLATE2,HUGE,,))

$(eval $(call PARTS_TEMPLATE2,XPRIM,HUGE,))

# Generate partial files for each XPRIM part (primorial only)
# Custom rules for XPRIM parts since they don't follow the standard decade/primorial pattern

# XPRIM parts use standard partial file architecture with dummy decade files
# Generate partial files for each XPRIM part using custom rules that follow the standard pattern

# XPRIM partial files are generated by the PARTS_TEMPLATE call above


$(CPS_SUMMARY_LPRIM).csv: $(CPS_SUMMARY_MPRIM).csv $(SUMMARY_DEFAULT_LPRIM).csv | $(MERGECPS)
	@set -Eeo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	$(MERGECPS) $(foreach a,$(CPS_SUMMARY_MPRIM).csv $(CPS_SUMMARY_LPRIM_PARTS),--input $(a)) --output "$@"

$(CPS_SUMMARY_XPRIM).csv: $(CPS_SUMMARY_LPRIM).csv $(SUMMARY_DEFAULT_XPRIM).csv | $(MERGECPS)
	@set -Eeo pipefail; trap 'echo "error at line $$LINENO" >&2; exit 1' ERR; \
	$(MERGECPS) $(foreach a,$(CPS_SUMMARY_LPRIM).csv $(CPS_SUMMARY_XPRIM_PARTS),--input $(a)) --output "$@"

# ---------- Generic sha256 rule ----------
%.sha256: %
	sha256sum "$<" | tee "$@"

# ---------- Top-level generation groups ----------
generate:  $(GBP) $(SGB_SMALL).csv $(SGB_SPRIM).csv $(SUMMARY_SMALL).csv \
	$(JOIN_SMALL).csv $(CPSLB_SMALL).csv $(CPSLB_SPRIM).csv $(LAVG_SMALL).csv $(LMIN_SMALL).csv \
	$(LMAX_SMALL).csv $(LSAVG_SMALL).csv $(LSMIN_SMALL).csv $(LSMAX_SMALL).csv

generate-medium: $(GBP) $(SGB_MEDIUM).csv $(SUMMARY_MEDIUM).csv \
	$(JOIN_MEDIUM).csv $(CPSLB_MEDIUM).csv $(CPSLB_MPRIM).csv  $(LAVG_MEDIUM).csv $(LMIN_MEDIUM).csv \
	$(LMAX_MEDIUM).csv $(LSAVG_MEDIUM).csv $(LSMIN_MEDIUM).csv $(LSMAX_MEDIUM).csv

generate-large: $(GBP) $(SGB_LARGE).csv $(SUMMARY_LARGE).csv \
	$(JOIN_LARGE).csv $(CPSLB_LARGE).csv $(CPSLB_LPRIM).csv $(LAVG_LARGE).csv $(LMIN_LARGE).csv \
	$(LMAX_LARGE).csv $(LSAVG_LARGE).csv $(LSMIN_LARGE).csv $(LSMAX_LARGE).csv

generate-huge: $(GBP) $(SGB_HUGE).csv $(SUMMARY_HUGE).csv \
	$(JOIN_HUGE).csv $(CPSLB_HUGE).csv $(LAVG_HUGE).csv $(LMIN_HUGE).csv \
	$(LMAX_HUGE).csv $(LSAVG_HUGE).csv $(LSMIN_HUGE).csv $(LSMAX_HUGE).csv \
	$(SGB_XPRIM).csv $(SUMMARY_XPRIM).csv $(JOIN_XPRIM).csv $(CPSLB_XPRIM).csv \
	$(LAVG_XPRIM).csv $(LMIN_XPRIM).csv \
	$(LSAVG_XPRIM).csv $(LSMIN_XPRIM).csv $(LSMAX_XPRIM).csv 

# ---------- Certification (base + per-size chains) ----------
BITMAP_VERIFY := $(BITMAP).verify
RAW_VERIFY    := $(RAW).verify
GBP_VERIFY    := $(GBP).verify

$(BITMAP_VERIFY): $(BITMAP) | $(CERTIFYPRIMES) $(OUT)
	./$(CERTIFYPRIMES) --bitmap --file "$(BITMAP)" | tee "$@"
	@(echo -n "sha256=" && sha256sum < "$(BITMAP)" | awk '{print $$1}') | tee -a "$@"

$(RAW_VERIFY): $(RAW) | $(CERTIFYPRIMES) $(OUT)
	./$(CERTIFYPRIMES) --binary --file "$(RAW)" | tee "$@"
	@(echo -n "sha256=" && sha256sum < "$(RAW)" | awk '{print $$1}') | tee -a "$@"

$(GBP_VERIFY): $(GBP) | $(CERTIFYGBPAIRS) $(OUT) $(BITMAP)
	./$(CERTIFYGBPAIRS) --bitmap "$(BITMAP)" --file "$(GBP)" | tee "$@"
	@(echo -n "sha256=" && sha256sum < "$(BITMAP)" | awk '{print $$1}') | tee -a "$@"

certify: $(BITMAP_VERIFY) $(RAW_VERIFY) $(GBP_VERIFY) \
	$(SGB_VERIFY_SMALL) \
	$(SUMMARY_VERIFY_SMALL) $(JOIN_VERIFY_SMALL) $(CPSLB_VERIFY_SMALL) $(CPS_SUMMARY_VERIFY_SMALL) \
	$(LAVG_VERIFY_SMALL) $(LMIN_VERIFY_SMALL) $(LMAX_VERIFY_SMALL) \
	$(LSAVG_VERIFY_SMALL) $(LSMIN_VERIFY_SMALL) $(LSMAX_VERIFY_SMALL) \
	$(SGB_VERIFY_SPRIM) \
	$(SUMMARY_VERIFY_SPRIM) $(JOIN_VERIFY_SPRIM) $(CPSLB_VERIFY_SPRIM) $(CPS_SUMMARY_VERIFY_SPRIM) \
	$(LAVG_VERIFY_SPRIM) $(LMIN_VERIFY_SPRIM) \
	$(LMAX_VERIFY_SPRIM) $(LSAVG_VERIFY_SPRIM) $(LSMIN_VERIFY_SPRIM) $(LSMAX_VERIFY_SPRIM)

certify-medium: \
	$(SGB_VERIFY_MEDIUM) $(SUMMARY_VERIFY_MEDIUM) \
	$(JOIN_VERIFY_MEDIUM) $(CPSLB_VERIFY_MEDIUM) $(CPS_SUMMARY_VERIFY_MEDIUM) \
	$(LAVG_VERIFY_MEDIUM) $(LMIN_VERIFY_MEDIUM) \
	$(LMAX_VERIFY_MEDIUM) $(LSAVG_VERIFY_MEDIUM) $(LSMIN_VERIFY_MEDIUM) $(LSMAX_VERIFY_MEDIUM) \
	$(SGB_VERIFY_MPRIM) $(SUMMARY_VERIFY_MPRIM) \
	$(JOIN_VERIFY_MPRIM) $(CPSLB_VERIFY_MPRIM) $(CPS_SUMMARY_VERIFY_MPRIM) \
	$(LAVG_VERIFY_MPRIM) $(LMIN_VERIFY_MPRIM) \
	$(LMAX_VERIFY_MPRIM) $(LSAVG_VERIFY_MPRIM) $(LSMIN_VERIFY_MPRIM) $(LSMAX_VERIFY_MPRIM) \
	certify

certify-large: \
	$(SGB_VERIFY_LARGE) $(SUMMARY_VERIFY_LARGE) \
	$(JOIN_VERIFY_LARGE) $(CPSLB_VERIFY_LARGE) $(CPS_SUMMARY_VERIFY_LARGE) \
	$(LAVG_VERIFY_LARGE) $(LMIN_VERIFY_LARGE) \
	$(LMAX_VERIFY_LARGE) $(LSAVG_VERIFY_LARGE) $(LSMIN_VERIFY_LARGE) $(LSMAX_VERIFY_LARGE) \
	$(SGB_VERIFY_LPRIM) $(SUMMARY_VERIFY_LPRIM) \
	$(JOIN_VERIFY_LPRIM) $(CPSLB_VERIFY_LPRIM) $(CPS_SUMMARY_VERIFY_LPRIM) \
	$(LAVG_VERIFY_LPRIM) $(LMIN_VERIFY_LPRIM) \
	$(LMAX_VERIFY_LPRIM) $(LSAVG_VERIFY_LPRIM) $(LSMIN_VERIFY_LPRIM) $(LSMAX_VERIFY_LPRIM) \
	certify-medium

certify-huge: \
	$(SGB_VERIFY_XPRIM) $(SUMMARY_VERIFY_XPRIM) \
	$(JOIN_VERIFY_XPRIM) $(CPSLB_VERIFY_XPRIM) $(CPS_SUMMARY_VERIFY_XPRIM) \
	$(LAVG_VERIFY_XPRIM) $(LMIN_VERIFY_XPRIM) \
	$(LMAX_VERIFY_XPRIM) $(LSAVG_VERIFY_XPRIM) $(LSMIN_VERIFY_XPRIM) $(LSMAX_VERIFY_XPRIM) \
	certify-large

# ---------- Compare against golden references ----------
.PHONY: verify verify-medium verify-large verify-huge validate validate-medium validate-large validate-huge

verify: certify 
	@(cmp "$(BITMAP_VERIFY)" "$(BITMAP_GOLD)" && echo "Validated $(BITMAP)") || test "$$TAINTED" = "1"
	@(cmp "$(RAW_VERIFY)"    "$(RAW_GOLD)" && echo "Validated $(RAW)") || test "$$TAINTED" = "1"
	@(cmp "$(GBP_VERIFY)"    "$(GBP_GOLD)" && echo "Validated $(GBP)") || test "$$TAINTED" = "1"
	@(cmp "$(SGB_SMALL).csv.verify" "$(SGB_GOLD_SMALL)" && echo "Validated $(SGB_SMALL).csv") || test "$$TAINTED" = "1"
	@(cmp "$(SUMMARY_SMALL).csv.verify" "$(SUMMARY_GOLD_SMALL)" && echo "Validated $(SUMMARY_SMALL).csv") || test "$$TAINTED" = "1"
#	@(cmp "$(CPS_SUMMARY_VERIFY_SMALL)" "$(CPS_SUMMARY_GOLD_SMALL)" && echo "Validated $(CPS_SUMMARY_SMALL).csv") || test "$$TAINTED" = "1"
	@(cmp "$(JOIN_VERIFY_SMALL)" "$(JOIN_GOLD_SMALL)" && echo "Validated $(JOIN_SMALL).csv") || test "$$TAINTED" = "1"
	@(cmp "$(CPSLB_VERIFY_SMALL)"   "$(CPSLB_GOLD_SMALL)" && echo "Validated $(CPSLB_SMALL).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LAVG_VERIFY_SMALL)"    "$(LAVG_GOLD_SMALL)" && echo "Validated $(LAVG_SMALL).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LMIN_VERIFY_SMALL)"    "$(LMIN_GOLD_SMALL)" && echo "Validated $(LMIN_SMALL).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LMAX_VERIFY_SMALL)"    "$(LMAX_GOLD_SMALL)" && echo "Validated $(LMAX_SMALL).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSAVG_VERIFY_SMALL)"    "$(LSAVG_GOLD_SMALL)" && echo "Validated $(LSAVG_SMALL).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSMIN_VERIFY_SMALL)"    "$(LSMIN_GOLD_SMALL)" && echo "Validated $(LSMIN_SMALL).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSMAX_VERIFY_SMALL)"    "$(LSMAX_GOLD_SMALL)" && echo "Validated $(LSMAX_SMALL).csv") || test "$$TAINTED" = "1"
	@(cmp "$(SGB_SPRIM).csv.verify"    "$(SGB_GOLD_SPRIM)" && echo "Validated $(SGB_SPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(SUMMARY_SPRIM).csv.verify" "$(SUMMARY_GOLD_SPRIM)" && echo "Validated $(SUMMARY_SPRIM).csv") || test "$$TAINTED" = "1"
#	@cmp "$(CPS_SUMMARY_VERIFY_SPRIM)" "$(CPS_SUMMARY_GOLD_SPRIM)" && echo "Validated $(CPS_SUMMARY_SPRIM).csv"
	@(cmp "$(JOIN_VERIFY_SPRIM)" "$(JOIN_GOLD_SPRIM)" && echo "Validated $(JOIN_SPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(CPSLB_VERIFY_SPRIM)"   "$(CPSLB_GOLD_SPRIM)" && echo "Validated $(CPSLB_SPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LAVG_VERIFY_SPRIM)"    "$(LAVG_GOLD_SPRIM)" && echo "Validated $(LAVG_SPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LMIN_VERIFY_SPRIM)"    "$(LMIN_GOLD_SPRIM)" && echo "Validated $(LMIN_SPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LMAX_VERIFY_SPRIM)"    "$(LMAX_GOLD_SPRIM)" && echo "Validated $(LMAX_SPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSAVG_VERIFY_SPRIM)"    "$(LSAVG_GOLD_SPRIM)" && echo "Validated $(LSAVG_SPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSMIN_VERIFY_SPRIM)"    "$(LSMIN_GOLD_SPRIM)" && echo "Validated $(LSMIN_SPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSMAX_VERIFY_SPRIM)"    "$(LSMAX_GOLD_SPRIM)" && echo "Validated $(LSMAX_SPRIM).csv") || test "$$TAINTED" = "1"

verify-medium: $(OUT)/verify-medium-$(COMPAT).stamp
	@echo "Medium validation completed successfully!"

$(OUT)/verify-medium-$(COMPAT).stamp: certify-medium verify
	@(cmp "$(SGB_MEDIUM).csv.verify"    "$(SGB_GOLD_MEDIUM)" && echo "Validated $(SGB_MEDIUM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(SUMMARY_MEDIUM).csv.verify" "$(SUMMARY_GOLD_MEDIUM)" && echo "Validated $(SUMMARY_MEDIUM).csv") || test "$$TAINTED" = "1"
#	@(cmp "$(CPS_SUMMARY_VERIFY_MEDIUM)" "$(CPS_SUMMARY_GOLD_MEDIUM)" && echo "Validated $(CPS_SUMMARY_MEDIUM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(JOIN_VERIFY_MEDIUM)"   "$(JOIN_GOLD_MEDIUM)" && echo "Validated $(JOIN_MEDIUM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(CPSLB_VERIFY_MEDIUM)"   "$(CPSLB_GOLD_MEDIUM)" && echo "Validated $(CPSLB_MEDIUM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LAVG_VERIFY_MEDIUM)"    "$(LAVG_GOLD_MEDIUM)" && echo "Validated $(LAVG_MEDIUM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LMIN_VERIFY_MEDIUM)"    "$(LMIN_GOLD_MEDIUM)" && echo "Validated $(LMIN_MEDIUM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LMAX_VERIFY_MEDIUM)"    "$(LMAX_GOLD_MEDIUM)" && echo "Validated $(LMAX_MEDIUM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSAVG_VERIFY_MEDIUM)"    "$(LSAVG_GOLD_MEDIUM)" && echo "Validated $(LSAVG_MEDIUM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSMIN_VERIFY_MEDIUM)"    "$(LSMIN_GOLD_MEDIUM)" && echo "Validated $(LSMIN_MEDIUM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSMAX_VERIFY_MEDIUM)"    "$(LSMAX_GOLD_MEDIUM)" && echo "Validated $(LSMAX_MEDIUM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(SUMMARY_MPRIM).csv.verify" "$(SUMMARY_GOLD_MPRIM)" && echo "Validated $(SUMMARY_MPRIM).csv") || test "$$TAINTED" = "1"
#	@(cmp "$(CPS_SUMMARY_VERIFY_MPRIM)" "$(CPS_SUMMARY_GOLD_MPRIM)" && echo "Validated $(CPS_SUMMARY_MPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(JOIN_VERIFY_MPRIM)"   "$(JOIN_GOLD_MPRIM)" && echo "Validated $(JOIN_MPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(CPSLB_VERIFY_MPRIM)"   "$(CPSLB_GOLD_MPRIM)" && echo "Validated $(CPSLB_MPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LAVG_VERIFY_MPRIM)"    "$(LAVG_GOLD_MPRIM)" && echo "Validated $(LAVG_MPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LMIN_VERIFY_MPRIM)"    "$(LMIN_GOLD_MPRIM)" && echo "Validated $(LMIN_MPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LMAX_VERIFY_MPRIM)"    "$(LMAX_GOLD_MPRIM)" && echo "Validated $(LMAX_MPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSAVG_VERIFY_MPRIM)"    "$(LSAVG_GOLD_MPRIM)" && echo "Validated $(LSAVG_MPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSMIN_VERIFY_MPRIM)"    "$(LSMIN_GOLD_MPRIM)" && echo "Validated $(LSMIN_MPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSMAX_VERIFY_MPRIM)"    "$(LSMAX_GOLD_MPRIM)" && echo "Validated $(LSMAX_MPRIM).csv") || test "$$TAINTED" = "1"
	@touch "$@"

verify-large: $(OUT)/verify-large-$(COMPAT).stamp
	@echo "Large validation completed successfully!"

$(OUT)/verify-large-$(COMPAT).stamp: certify-large verify-medium
	@(cmp "$(SGB_LARGE).csv.verify"    "$(SGB_GOLD_LARGE)" && echo "Validated $(SGB_LARGE).csv") || test "$$TAINTED" = "1"
	@(cmp "$(SUMMARY_LARGE).csv.verify" "$(SUMMARY_GOLD_LARGE)" && echo "Validated $(SUMMARY_LARGE).csv") || test "$$TAINTED" = "1"
#	@(cmp "$(CPS_SUMMARY_VERIFY_LARGE)" "$(CPS_SUMMARY_GOLD_LARGE)" && echo "Validated $(CPS_SUMMARY_LARGE).csv") || test "$$TAINTED" = "1"
	@(cmp "$(JOIN_VERIFY_LARGE)"   "$(JOIN_GOLD_LARGE)" && echo "Validated $(JOIN_LARGE).csv") || test "$$TAINTED" = "1"
	@(cmp "$(CPSLB_VERIFY_LARGE)"   "$(CPSLB_GOLD_LARGE)" && echo "Validated $(CPSLB_LARGE).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LAVG_VERIFY_LARGE)"    "$(LAVG_GOLD_LARGE)" && echo "Validated $(LAVG_LARGE).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LMIN_VERIFY_LARGE)"    "$(LMIN_GOLD_LARGE)" && echo "Validated $(LMIN_LARGE).csv") || test "$$TAINTED" = "1"	
	@(cmp "$(LMAX_VERIFY_LARGE)"    "$(LMAX_GOLD_LARGE)" && echo "Validated $(LMAX_LARGE).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSAVG_VERIFY_LARGE)"    "$(LSAVG_GOLD_LARGE)" && echo "Validated $(LSAVG_LARGE).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSMIN_VERIFY_LARGE)"    "$(LSMIN_GOLD_LARGE)" && echo "Validated $(LSMIN_LARGE).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSMAX_VERIFY_LARGE)"    "$(LSMAX_GOLD_LARGE)" && echo "Validated $(LSMAX_LARGE).csv") || test "$$TAINTED" = "1"
	@(cmp "$(SUMMARY_LPRIM).csv.verify" "$(SUMMARY_GOLD_LPRIM)" && echo "Validated $(SUMMARY_LPRIM).csv") || test "$$TAINTED" = "1"
#	@(cmp "$(CPS_SUMMARY_VERIFY_LPRIM)" "$(CPS_SUMMARY_GOLD_LPRIM)" && echo "Validated $(CPS_SUMMARY_LPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(JOIN_VERIFY_LPRIM)"   "$(JOIN_GOLD_LPRIM)" && echo "Validated $(JOIN_LPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(CPSLB_VERIFY_LPRIM)"   "$(CPSLB_GOLD_LPRIM)" && echo "Validated $(CPSLB_LPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LAVG_VERIFY_LPRIM)"    "$(LAVG_GOLD_LPRIM)" && echo "Validated $(LAVG_LPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LMIN_VERIFY_LPRIM)"    "$(LMIN_GOLD_LPRIM)" && echo "Validated $(LMIN_LPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LMAX_VERIFY_LPRIM)"    "$(LMAX_GOLD_LPRIM)" && echo "Validated $(LMAX_LPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSAVG_VERIFY_LPRIM)"    "$(LSAVG_GOLD_LPRIM)" && echo "Validated $(LSAVG_LPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSMIN_VERIFY_LPRIM)"    "$(LSMIN_GOLD_LPRIM)" && echo "Validated $(LSMIN_LPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSMAX_VERIFY_LPRIM)"    "$(LSMAX_GOLD_LPRIM)" && echo "Validated $(LSMAX_LPRIM).csv") || test "$$TAINTED" = "1"
	@touch "$@"

verify-huge: $(OUT)/verify-huge-$(COMPAT).stamp
	@echo "Huge validation completed successfully!"

$(OUT)/verify-huge-$(COMPAT).stamp: certify-huge verify-large
	@(cmp "$(SGB_XPRIM).csv.verify"    "$(SGB_GOLD_XPRIM)" && echo "Validated $(SGB_XPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(SUMMARY_XPRIM).csv.verify" "$(SUMMARY_GOLD_XPRIM)" && echo "Validated $(SUMMARY_XPRIM).csv") || test "$$TAINTED" = "1"
#	@(cmp "$(CPS_SUMMARY_VERIFY_XPRIM)" "$(CPS_SUMMARY_GOLD_XPRIM)" && echo "Validated $(CPS_SUMMARY_XPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(JOIN_VERIFY_XPRIM)"   "$(JOIN_GOLD_XPRIM)" && echo "Validated $(JOIN_XPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(CPSLB_VERIFY_XPRIM)"   "$(CPSLB_GOLD_XPRIM)" && echo "Validated $(CPSLB_XPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LAVG_VERIFY_XPRIM)"    "$(LAVG_GOLD_XPRIM)" && echo "Validated $(LAVG_XPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LMIN_VERIFY_XPRIM)"    "$(LMIN_GOLD_XPRIM)" && echo "Validated $(LMIN_XPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LMAX_VERIFY_XPRIM)"    "$(LMAX_GOLD_XPRIM)" && echo "Validated $(LMAX_XPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSAVG_VERIFY_XPRIM)"    "$(LSAVG_GOLD_XPRIM)" && echo "Validated $(LSAVG_XPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSMIN_VERIFY_XPRIM)"    "$(LSMIN_GOLD_XPRIM)" && echo "Validated $(LSMIN_XPRIM).csv") || test "$$TAINTED" = "1"
	@(cmp "$(LSMAX_VERIFY_XPRIM)"    "$(LSMAX_GOLD_XPRIM)" && echo "Validated $(LSMAX_XPRIM).csv") || test "$$TAINTED" = "1"
	@touch "$@"


validate: verify

validate-medium: verify-medium

validate-large: verify-large

validate-huge: verify-huge

# ==========================================================================


# ---------- Housekeeping ----------
.PHONY: all generate generate-medium generate-large generate-huge clean clean-small clean-medium clean-large clean-huge clobber clobber-small clobber-medium clobber-large clobber-huge touch touch-small touch-medium touch-large touch-huge
 
clean: clean-small clean-medium clean-large clean-huge
	@test ! -d "$(OUT)" || $(RM) "$(OUT)"/*.verify "$(OUT)"/*.sha256

clean-small: clean-$(SUFFIX_SMALL) clean-$(SUFFIX_SPRIM)

clean-medium: clean-$(SUFFIX_MEDIUM) clean-$(SUFFIX_MPRIM)

clean-large: clean-$(SUFFIX_LARGE) clean-$(SUFFIX_LPRIM)

clean-huge: clean-$(SUFFIX_HUGE) clean-$(SUFFIX_XPRIM)

ALL_OUTPUTS := $(BITMAP) $(RAW) $(GBP) \
               $(OUTPUT_SMALL) $(OUTPUT_MEDIUM) $(OUTPUT_LARGE) $(OUTPUT_HUGE) $(OUTPUT_XPRIM)

clobber: clean clobber-small clobber-medium clobber-large clobber-huge
	$(RM) $(ALL_OUTPUTS)
	$(MAKE) -C src clean

clobber-small: clobber-$(SUFFIX_SMALL) clobber-$(SUFFIX_SPRIM)

clobber-medium: clobber-$(SUFFIX_MEDIUM) clobber-$(SUFFIX_MPRIM)

clobber-large: clobber-$(SUFFIX_LARGE) clobber-$(SUFFIX_LPRIM)

clobber-huge: clean-huge clobber-$(SUFFIX_HUGE) clobber-$(SUFFIX_XPRIM)

touch: touch-small touch-medium touch-large touch-huge
	$(RM) $(ALL_OUTPUTS)
	$(MAKE) -C src clean

touch-small: touch-$(SUFFIX_SMALL) touch-$(SUFFIX_SPRIM)

touch-medium: touch-$(SUFFIX_MEDIUM) touch-$(SUFFIX_MPRIM)

touch-large: touch-$(SUFFIX_LARGE) touch-$(SUFFIX_LPRIM)

touch-huge: touch-$(SUFFIX_HUGE) touch-$(SUFFIX_XPRIM)

.PHONY: help
help:
	@echo "Default: validate-skip-summary-ml (generate small; use data/ for medium+large summaries)"
	@echo "Common:"
	@echo "  make validate            # full validate (generate all summaries)"
	@echo "  make validate-medium     # validate through medium"
	@echo "  make validate-large      # validate through large"
	@echo "  make validate-huge       # validate through huge"
	@echo "  make validate-skip-summary-ml  # generate SGB all sizes, summary small only"
	@echo "  make clean-small         # clean small size files (1M, 17PR2)"
	@echo "  make clean-medium        # clean medium size files (10M, 19PR)"
	@echo "  make clean-large         # clean large size files (100M, 23PR)"
	@echo "  make clean-huge          # clean huge size files (XPRIM parts)"
	@echo "  make clobber             # remove output artifacts; keep data/"
	@echo "  make clobber-small       # remove small data files (1M, 17PR2)"
	@echo "  make clobber-medium      # remove medium data files (10M, 19PR)"
	@echo "  make clobber-large       # remove large data files (100M, 23PR)"
	@echo "  make clobber-huge        # remove huge data files (XPRIM parts)"


# ---- Paper Source Archive Configuration ------------------------
DATE := $(shell date +%Y%m%d)

# List all papers (add new papers here)
ALL_PAPERS := goldbach-reformulation
# Example: ALL_PAPERS := goldbach-reformulation another-paper third-paper

# Define files for each paper
GOLDBACH_REFORMULATION_FILES := \
  LICENSES/CC-BY-4.0.txt \
  papers/goldbach-reformulation/sieve_goldbach.tex \
  papers/goldbach-reformulation/sieve_goldbach.bib \
  papers/goldbach-reformulation/README.txt \
  papers/goldbach-reformulation/Makefile \
  papers/goldbach-reformulation/cpslowerbound-100M.csv \
  papers/goldbach-reformulation/lambdaavg-100M.csv \
  papers/goldbach-reformulation/lambdamax-100M.csv \
  papers/goldbach-reformulation/lambdamin-100M.csv \
  papers/goldbach-reformulation/pairrangejoin-100M.csv

# Add more papers here as needed:
# ANOTHER_PAPER_FILES := \
#   LICENSES/CC-BY-4.0.txt \
#   papers/another-paper/main.tex \
#   ...

.PHONY: zip-goldbach-reformulation clean-goldbach-reformulation

# ---- Build the date-stamped source archive for goldbach-reformulation -----
zip-goldbach-reformulation:
	@echo "Creating source archive for goldbach-reformulation..."
	@rm -rf "$(OUT)/goldbach-reformulation-src"
	@mkdir -p "$(OUT)/goldbach-reformulation-src"
	@cp -f $(GOLDBACH_REFORMULATION_FILES) "$(OUT)/goldbach-reformulation-src/."
	@cd "$(OUT)/goldbach-reformulation-src" && ( \
	  echo "SHA256 manifest for goldbach-reformulation ($(DATE))" > CHECKSUMS.txt; \
	  for f in $(notdir $(GOLDBACH_REFORMULATION_FILES)); do sha256sum "$$f" >> CHECKSUMS.txt 2>/dev/null || shasum -a 256 "$$f" >> CHECKSUMS.txt; done )
	@(cd "$(OUT)" && zip -r -X "goldbach-reformulation-src-$(DATE).zip" "goldbach-reformulation-src" >/dev/null)
	@rm -rf "$(OUT)/goldbach-reformulation-src"
	@echo "Created $(OUT)/goldbach-reformulation-src-$(DATE).zip"
	@(cd "$(OUT)" && shasum -a 256 "goldbach-reformulation-src-$(DATE).zip" | tee "goldbach-reformulation-src-$(DATE).sha256")

# ---- Cleanup goldbach-reformulation archive ----------------------------
clean-goldbach-reformulation:
	@rm -rf "$(OUT)/goldbach-reformulation-src-$(DATE).zip" "$(OUT)/goldbach-reformulation-src-$(DATE).sha256"

# ---- Build/clean all papers ----------------------------------------
.PHONY: zip-src clean-src
zip-src:
	@echo "Building source archives for all papers..."
	@for paper in $(ALL_PAPERS); do \
		$(MAKE) zip-$$paper; \
	done
	@echo "All paper source archives created."

clean-src:
	@echo "Cleaning source archives for all papers..."
	@for paper in $(ALL_PAPERS); do \
		$(MAKE) clean-$$paper; \
	done
	@echo "All paper source archives cleaned."

test-touch: output/test-touch.stamp
output/test-touch.stamp:
	@echo "Creating test file"
	@touch "$@"D
	@touch "$@"D
