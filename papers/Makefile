OUT=../output

# Default target: build all papers
.DEFAULT_GOAL := all

# ---- Paper Source Archive Configuration ------------------------
DATE := $(shell date +%Y%m%d)

# List all papers (add new papers here)
ALL_PAPERS := goldbach-reformulation pcgc-goldbach-derivation pcgc-goldbach-empirical pcgc-goldbach-reductions
# Example: ALL_PAPERS := goldbach-reformulation another-paper third-paper

# ---- Build all papers ----------------------------------------
.PHONY: all
all:
	@echo "Building all papers..."
	@for paper in $(ALL_PAPERS); do \
		echo "Building $$paper..."; \
		$(MAKE) -C $$paper all || exit 1; \
	done
	@echo "All papers built successfully."

# Helper function to generate file list for a paper
# Usage: $(call PAPER_FILES,paper-name,tex-name)
# tex-name is optional, defaults to paper-name if not provided
define PAPER_FILES
  ../LICENSES/CC-BY-4.0.txt \
  $(1)/$(if $(2),$(2),$(1)).tex \
  $(1)/$(if $(2),$(2),$(1)).bib \
  $(1)/Makefile \
  $(wildcard $(1)/*.csv)
endef

# Define files for each paper
# goldbach-reformulation uses standard naming
GOLDBACH_REFORMULATION_FILES := $(call PAPER_FILES,goldbach-reformulation) \
  goldbach-reformulation/README.txt

# pcgc-goldbach-derivation uses standard naming
PCGC_GOLDBACH_DERIVATION_FILES := $(call PAPER_FILES,pcgc-goldbach-derivation)

# pcgc-goldbach-empirical uses standard naming
PCGC_GOLDBACH_EMPIRICAL_FILES := $(call PAPER_FILES,pcgc-goldbach-empirical)

# pcgc-goldbach-empirical uses standard naming
PCGC_GOLDBACH_REDUCTIONS_FILES := $(call PAPER_FILES,pcgc-goldbach-reductions)

.PHONY: zip-goldbach-reformulation clean-goldbach-reformulation zip-pcgc-goldbach-derivation clean-pcgc-goldbach-derivation zip-pcgc-goldbach-empirical clean-pcgc-goldbach-empirical zip-pcgc-goldbach-reductions clean-pcgc-goldbach-reductions 

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

# ---- Build the date-stamped source archive for pcgc-goldbach-derivation -----
zip-pcgc-goldbach-derivation:
	@echo "Creating source archive for pcgc-goldbach-derivation..."
	@rm -rf "$(OUT)/pcgc-goldbach-derivation-src"
	@mkdir -p "$(OUT)/pcgc-goldbach-derivation-src"
	@cp -f $(PCGC_GOLDBACH_DERIVATION_FILES) "$(OUT)/pcgc-goldbach-derivation-src/."
	@cd "$(OUT)/pcgc-goldbach-derivation-src" && ( \
	  echo "SHA256 manifest for pcgc-goldbach-derivation ($(DATE))" > CHECKSUMS.txt; \
	  for f in $(notdir $(PCGC_GOLDBACH_DERIVATION_FILES)); do sha256sum "$$f" >> CHECKSUMS.txt 2>/dev/null || shasum -a 256 "$$f" >> CHECKSUMS.txt; done )
	@(cd "$(OUT)" && zip -r -X "pcgc-goldbach-derivation-src-$(DATE).zip" "pcgc-goldbach-derivation-src" >/dev/null)
	@rm -rf "$(OUT)/pcgc-goldbach-derivation-src"
	@echo "Created $(OUT)/pcgc-goldbach-derivation-src-$(DATE).zip"
	@(cd "$(OUT)" && shasum -a 256 "pcgc-goldbach-derivation-src-$(DATE).zip" | tee "pcgc-goldbach-derivation-src-$(DATE).sha256")

# ---- Cleanup pcgc-goldbach-derivation archive ----------------------------
clean-pcgc-goldbach-derivation:
	@rm -rf "$(OUT)/pcgc-goldbach-derivation-src-$(DATE).zip" "$(OUT)/pcgc-goldbach-derivation-src-$(DATE).sha256"

# ---- Build the date-stamped source archive for pcgc-goldbach-empirical -----
zip-pcgc-goldbach-empirical:
	@echo "Creating source archive for pcgc-goldbach-empirical..."
	@rm -rf "$(OUT)/pcgc-goldbach-empirical-src"
	@mkdir -p "$(OUT)/pcgc-goldbach-empirical-src"
	@cp -f $(PCGC_GOLDBACH_EMPIRICAL_FILES) "$(OUT)/pcgc-goldbach-empirical-src/."
	@cd "$(OUT)/pcgc-goldbach-empirical-src" && ( \
	  echo "SHA256 manifest for pcgc-goldbach-empirical ($(DATE))" > CHECKSUMS.txt; \
	  for f in $(notdir $(PCGC_GOLDBACH_EMPIRICAL_FILES)); do sha256sum "$$f" >> CHECKSUMS.txt 2>/dev/null || shasum -a 256 "$$f" >> CHECKSUMS.txt; done )
	@(cd "$(OUT)" && zip -r -X "pcgc-goldbach-empirical-src-$(DATE).zip" "pcgc-goldbach-empirical-src" >/dev/null)
	@rm -rf "$(OUT)/pcgc-goldbach-empirical-src"
	@echo "Created $(OUT)/pcgc-goldbach-empirical-src-$(DATE).zip"
	@(cd "$(OUT)" && shasum -a 256 "pcgc-goldbach-empirical-src-$(DATE).zip" | tee "pcgc-goldbach-empirical-src-$(DATE).sha256")

# ---- Cleanup pcgc-goldbach-empirical archive ----------------------------
clean-pcgc-goldbach-empirical:
	@rm -rf "$(OUT)/pcgc-goldbach-empirical-src-$(DATE).zip" "$(OUT)/pcgc-goldbach-empirical-src-$(DATE).sha256"

# ---- Build the date-stamped source archive for pcgc-goldbach-reductions -----
zip-pcgc-goldbach-reductions:
	@echo "Creating source archive for pcgc-goldbach-reductions..."
	@rm -rf "$(OUT)/pcgc-goldbach-reductions-src"
	@mkdir -p "$(OUT)/pcgc-goldbach-reductions-src"
	@cp -f $(PCGC_GOLDBACH_REDUCTIONS_FILES) "$(OUT)/pcgc-goldbach-reductions-src/."
	@cd "$(OUT)/pcgc-goldbach-reductions-src" && ( \
	  echo "SHA256 manifest for pcgc-goldbach-reductions ($(DATE))" > CHECKSUMS.txt; \
	  for f in $(notdir $(PCGC_GOLDBACH_REDUCTIONS_FILES)); do sha256sum "$$f" >> CHECKSUMS.txt 2>/dev/null || shasum -a 256 "$$f" >> CHECKSUMS.txt; done )
	@(cd "$(OUT)" && zip -r -X "pcgc-goldbach-reductions-src-$(DATE).zip" "pcgc-goldbach-reductions-src" >/dev/null)
	@rm -rf "$(OUT)/pcgc-goldbach-reductions-src"
	@echo "Created $(OUT)/pcgc-goldbach-reductions-src-$(DATE).zip"
	@(cd "$(OUT)" && shasum -a 256 "pcgc-goldbach-reductions-src-$(DATE).zip" | tee "pcgc-goldbach-reductions-src-$(DATE).sha256")

# ---- Cleanup pcgc-goldbach-reductions archive ----------------------------
clean-pcgc-goldbach-reductions:
	@rm -rf "$(OUT)/pcgc-goldbach-reductions-src-$(DATE).zip" "$(OUT)/pcgc-goldbach-reductions-src-$(DATE).sha256"


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

