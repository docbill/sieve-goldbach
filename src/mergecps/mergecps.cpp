// mergecps - merges CPS summary files from multiple runs
// Copyright (C) 2025 Bill C. Riemers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdint>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <getopt.h>

struct CPSRow {
    std::uint64_t n_start;
    std::uint64_t n_end;
    long double alpha;
    std::uint64_t preMertens;
    std::uint64_t nstar;
    long double deltaMertens;
    std::uint64_t n_5percent;
    std::uint64_t nzeroStat;
    long double etaStat;
    std::uint64_t nstarAsymp;
    long double deltaMertensAsymp;
    std::uint64_t nzeroStatAsymp;
    long double etaStatAsymp;
    
    std::string source_file;
    
    CPSRow() = default;
    
    CPSRow( std::uint64_t ns, std::uint64_t ne, long double a, std::uint64_t pm,
            std::uint64_t ns_val, long double dm, std::uint64_t n5p,
            std::uint64_t nzs, long double es, std::uint64_t nsa,
            long double dma, std::uint64_t nzsa, long double esa,
            const std::string& src = "")
        : n_start(ns), n_end(ne), alpha(a), preMertens(pm), nstar(ns_val),
            deltaMertens(dm), n_5percent(n5p), nzeroStat(nzs), etaStat(es),
            nstarAsymp(nsa), deltaMertensAsymp(dma), nzeroStatAsymp(nzsa),
            etaStatAsymp(esa), source_file(src) {}
};

struct OverlapInfo {
    std::uint64_t start_n, end_n;
    long double alpha;
    std::vector<std::string> conflicting_files;
    std::vector<CPSRow> conflicting_rows;
};

struct GapInfo {
    std::uint64_t start_n, end_n;
    long double alpha;
};

class CPSMerger {
private:
    std::vector<CPSRow> all_rows;
    std::map<long double, std::vector<CPSRow>> alpha_groups;
    
public:
    
    void loadCPSFile(const std::string& filename) {
        FILE* file = std::fopen(filename.c_str(), "r");
        if (!file) {
            throw std::runtime_error("Cannot open file: " + filename);
        }
        
        char line[1024];
        int lineNum = 0;
        
        // Skip header line if it exists
        if (std::fgets(line, sizeof(line), file)) {
            lineNum++;
            // Check if this looks like a header (contains "FIRST" or "Alpha")
            if (strstr(line, "FIRST") || strstr(line, "Alpha")) {
                // This is a header, continue to next line
            } else {
                // This is data, rewind to beginning
                std::rewind(file);
                lineNum = 0;
            }
        }
        
        while (std::fgets(line, sizeof(line), file)) {
            lineNum++;
            
            // Skip empty lines
            if (line[0] == '\n' || line[0] == '\r') {
                continue;
            }
            
            // Parse the line
            std::uint64_t n_start, n_end, preMertens, nstar, n_5percent, nzeroStat, nstarAsymp, nzeroStatAsymp;
            long double alpha, deltaMertens, etaStat, deltaMertensAsymp, etaStatAsymp;
            
            int parsed = std::sscanf(line, "%" SCNu64 ",%" SCNu64 ",%Lf,%" SCNu64 ",%" SCNu64 ",%Lf,%" SCNu64 ",%" SCNu64 ",%Lf,%" SCNu64 ",%Lf,%" SCNu64 ",%Lf",
                &n_start, &n_end, &alpha, &preMertens,
                &nstar, &deltaMertens, &n_5percent, &nzeroStat, &etaStat,
                &nstarAsymp, &deltaMertensAsymp, &nzeroStatAsymp, &etaStatAsymp
            );
            
            if (parsed != 13) {
                std::fprintf(stderr, "ERROR: Malformed line %d in %s - expected 13 fields, got %d\n", lineNum, filename.c_str(), parsed);
                std::fclose(file);
                throw std::runtime_error("Malformed input data detected");
            }
            
            all_rows.emplace_back(n_start, n_end, alpha, preMertens, nstar, deltaMertens,
                                n_5percent, nzeroStat, etaStat, nstarAsymp, deltaMertensAsymp,
                                nzeroStatAsymp, etaStatAsymp, filename);
        }
        
        std::fclose(file);
        std::printf("Loaded %zu rows from %s\n", all_rows.size(), filename.c_str());
    }
    
    void groupByAlpha() {
        for (const auto& row : all_rows) {
            alpha_groups[row.alpha].push_back(row);
        }
        
        // Sort each alpha group by n_start
        for (auto& [alpha, rows] : alpha_groups) {
            std::sort(rows.begin(), rows.end(), 
                [](const CPSRow& a, const CPSRow& b) {
                        return a.n_start < b.n_start;
                });
        }
    }
    
    std::vector<OverlapInfo> detectOverlaps() {
        std::vector<OverlapInfo> overlaps;
        
        for (auto& [alpha, rows] : alpha_groups) {
            for (size_t i = 0; i < rows.size(); i++) {
                for (size_t j = i + 1; j < rows.size(); j++) {
                    const auto& row1 = rows[i];
                    const auto& row2 = rows[j];
                    
                    // Check for overlap
                    if (row1.n_end > row2.n_start) {
                        OverlapInfo overlap;
                        overlap.alpha = alpha;
                        overlap.start_n = std::max(row1.n_start, row2.n_start);
                        overlap.end_n = std::min(row1.n_end, row2.n_end);
                        overlap.conflicting_files = {row1.source_file, row2.source_file};
                        overlap.conflicting_rows = {row1, row2};
                        overlaps.push_back(overlap);
                    }
                }
            }
        }
        
        return overlaps;
    }
    
    std::vector<GapInfo> detectGaps() {
        std::vector<GapInfo> gaps;
        
        for (auto& [alpha, rows] : alpha_groups) {
            if (rows.empty()) continue;
            
            for (size_t i = 0; i < rows.size() - 1; i++) {
                const auto& current = rows[i];
                const auto& next = rows[i + 1];
                
                if (current.n_end < next.n_start) {
                    GapInfo gap;
                    gap.alpha = alpha;
                    gap.start_n = current.n_end;
                    gap.end_n = next.n_start;
                    gaps.push_back(gap);
                }
            }
        }
        
        return gaps;
    }
    
    
    void mergeAlphaGroup(const std::vector<CPSRow>& rows, FILE* output) {
        if (rows.empty()) return;
        
        // Group continuous intervals for merging
        std::vector<std::vector<CPSRow>> continuous_groups;
        std::vector<CPSRow> current_group;
        
        for (size_t i = 0; i < rows.size(); i++) {
            const auto& row = rows[i];
            
            if (current_group.empty()) {
                // First row
                current_group.push_back(row);
            } else {
                const auto& prev_row = current_group.back();
                
                // Check if this row is continuous with the previous one
                if (prev_row.n_end == row.n_start) {
                    // Continuous - add to current group
                    current_group.push_back(row);
                } else {
                    // Gap detected - finish current group and start new one
                    continuous_groups.push_back(current_group);
                    current_group.clear();
                    current_group.push_back(row);
                }
            }
        }
        
        // Don't forget the last group
        if (!current_group.empty()) {
            continuous_groups.push_back(current_group);
        }
        
        // Merge each continuous group
        for (const auto& group : continuous_groups) {
            if (group.empty()) continue;
            
            const auto& first_row = group[0];
            const auto& last_row = group.back();
            
            // Handle preMertens inheritance
            std::uint64_t effective_preMertens = first_row.n_start - 1;
            for (const auto& row : group) {
                if (row.preMertens >= row.n_start || ! row.preMertens) {
                    effective_preMertens = row.preMertens;
                }
            }

            // Find n_5percent: first row with non-zero value
            std::uint64_t merged_n_5percent = 0;
            for (const auto& row : group) {
                if (row.n_5percent > 0) {
                    merged_n_5percent = row.n_5percent;
                    break;
                }
            }

            std::uint64_t merged_nstar = 0;
            long double merged_deltaMertens = 0.0L;
            std::uint64_t merged_nstarAsymp = 0;
            long double merged_deltaMertensAsymp = 0.0L;
            long double merged_etaStat = 0.0L;
            std::uint64_t merged_nzeroStat = 0;
            long double merged_etaStatAsymp = 0.0L;
            std::uint64_t merged_nzeroStatAsymp = 0;
            if(effective_preMertens >= first_row.n_start) {
                // Find nstar: lowest value above effective_preMertens
                for (const auto& row : group) {
                    if (row.nstar > effective_preMertens && ! merged_nstar) {
                        merged_nstar = row.nstar;
                        merged_deltaMertens = row.deltaMertens;
                        break;
                    }
                }
                // Find nstarAsymp: lowest value above effective_preMertens
                for (const auto& row : group) {
                    if (row.nstarAsymp > effective_preMertens && ! merged_nstarAsymp) {
                        merged_nstarAsymp = row.nstarAsymp;
                        merged_deltaMertensAsymp = row.deltaMertensAsymp;
                        break;
                    }
                }
            
                if(merged_n_5percent >= first_row.n_start) {
                    for (const auto& row : group) {
                        // Find etaStat: lowest positive value where nzeroStat > n_5percent
                        if (row.etaStat > 0.0L && row.nzeroStat > merged_n_5percent && row.nzeroStat > effective_preMertens)  {
                            merged_etaStat = row.etaStat;
                            merged_nzeroStat = row.nzeroStat;
                        }
                        // Find etaStatAsymp: lowest positive value where nzeroStatAsymp > n_5percent
                        if (row.etaStatAsymp > 0.0L && row.nzeroStatAsymp > merged_n_5percent && row.nzeroStatAsymp > effective_preMertens) {
                            merged_etaStatAsymp = row.etaStatAsymp;
                            merged_nzeroStatAsymp = row.nzeroStatAsymp;
                        }
                    }
                }
            }
            
            // Write merged row
            std::fprintf(output, "%" PRIu64 ",%" PRIu64 ",%.12Lg,%" PRIu64 ",%" PRIu64 ",%.6Lf,%" PRIu64 ",%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf,%" PRIu64 ",%.6Lf\n",
                first_row.n_start, last_row.n_end, first_row.alpha, effective_preMertens,
                merged_nstar, merged_deltaMertens, merged_n_5percent, merged_nzeroStat, merged_etaStat,
                merged_nstarAsymp, merged_deltaMertensAsymp, merged_nzeroStatAsymp, merged_etaStatAsymp);
        }
    }
    
    void merge(const std::string& outputFile) {
        groupByAlpha();
        
        // Detect overlaps and gaps
        auto overlaps = detectOverlaps();
        auto gaps = detectGaps();
        
        // Report overlaps (throw error)
        if (!overlaps.empty()) {
            std::fprintf(stderr, "ERROR: Found %zu overlapping ranges:\n", overlaps.size());
            for (const auto& overlap : overlaps) {
                std::fprintf(stderr, "  Alpha %.12Lg: n=%" PRIu64 "-%" PRIu64 " conflicts between files: %s\n",
                    overlap.alpha, overlap.start_n, overlap.end_n,
                    overlap.conflicting_files[0].c_str());
                for (size_t i = 1; i < overlap.conflicting_files.size(); i++) {
                    std::fprintf(stderr, "    and %s\n", overlap.conflicting_files[i].c_str());
                }
            }
            throw std::runtime_error("Overlapping ranges detected. Please resolve manually.");
        }
        
        // Report gaps as warnings (merge will proceed with multiple rows)
        if (!gaps.empty()) {
            std::fprintf(stderr, "WARNING: Found %zu gaps - merge will produce multiple rows:\n", gaps.size());
            for (const auto& gap : gaps) {
                std::fprintf(stderr, "  Alpha %.12Lg: gap at n=%" PRIu64 "-%" PRIu64 "\n",
                    gap.alpha, gap.start_n, gap.end_n);
            }
        }
        
        // Merge and write output
        FILE* output = std::fopen(outputFile.c_str(), "w");
        if (!output) {
            throw std::runtime_error("Cannot open output file: " + outputFile);
        }
        
        // Write header
        std::fprintf(output, "FIRST,LAST,Alpha,PreMertens,Mertens,DeltaMertens,n_5precent,NzeroStat,EtaStat,MertensAsymp,DeltaMertensAsymp,NzeroStatAsymp,EtaStatAsymp\n");
        
        // Merge each alpha group
        for (auto& [alpha, rows] : alpha_groups) {
            mergeAlphaGroup(rows, output);
        }
        
        std::fclose(output);
        std::printf("Merged %zu rows into %s\n", all_rows.size(), outputFile.c_str());
    }
};

void printUsage(const char* program) {
    std::fprintf(stderr, "Usage: %s [OPTIONS] --input FILE1 [FILE2 ...] --output FILE\n", program);
    std::fprintf(stderr, "  --input FILE     Input CPS summary file (can be specified multiple times)\n");
    std::fprintf(stderr, "  --output FILE    Output merged CPS summary file\n");
    std::fprintf(stderr, "  --help           Show this help message\n");
}

int main(int argc, char* argv[]) {
    std::vector<std::string> inputFiles;
    std::string outputFile;
    
    static struct option long_opts[] = {
        {"input",   required_argument, 0, 'i'},
        {"output",  required_argument, 0, 'o'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt, opt_index;
    while ((opt = getopt_long(argc, argv, "i:o:h", long_opts, &opt_index)) != -1) {
        switch (opt) {
            case 'i':
                inputFiles.push_back(optarg);
                break;
            case 'o':
                outputFile = optarg;
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }
    
    if (inputFiles.empty() || outputFile.empty()) {
        std::fprintf(stderr, "Error: Both --input and --output are required\n");
        printUsage(argv[0]);
        return 1;
    }
    
    try {
        CPSMerger merger;
        
        // Load all input files
        for (const auto& file : inputFiles) {
            merger.loadCPSFile(file);
        }
        
        // Merge and write output
        merger.merge(outputFile);
        
        return 0;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Error: %s\n", e.what());
        return 1;
    }
}
