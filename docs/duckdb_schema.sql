-- DuckDB Schema for Aggregated Lambda Data
-- Write once, read many times for analytical queries

-- Main table: aggregated intervals (O(√n) records)
-- Uses primorial-based binning with support for partial bins
CREATE TABLE lambda_intervals (
    -- Interval identification
    interval_id BIGINT PRIMARY KEY,
    n_start BIGINT NOT NULL,
    n_end BIGINT NOT NULL,
    interval_size BIGINT NOT NULL,  -- n_end - n_start
    
    -- Primorial-based binning
    -- Note: bin_primorial may be smaller than analysis_primorial for O(√n) storage
    -- Example: bin_primorial = sqrt(19#/2), analysis_primorial = 19#
    bin_primorial_base TEXT NOT NULL,       -- e.g., "17#", the primorial used for bin size
    bin_primorial_value BIGINT NOT NULL,    -- actual bin primorial value (smaller, for O(√n) bins)
    analysis_primorial_base TEXT NOT NULL,  -- e.g., "19#", the primorial being analyzed
    analysis_primorial_value BIGINT NOT NULL, -- actual analysis primorial value
    bin_index BIGINT NOT NULL,              -- index of bin within bin_primorial period
    bin_start_offset BIGINT NOT NULL,       -- offset from bin_primorial_value * bin_index
    bin_end_offset BIGINT NOT NULL,         -- offset from bin_primorial_value * bin_index
    is_partial BOOLEAN DEFAULT FALSE,       -- true if this is a partial bin
    bin_completeness DOUBLE DEFAULT 1.0,    -- fraction of full bin (1.0 = complete, <1.0 = partial)
    -- Partial bins allow rebinning to larger bins (e.g., decade-based or larger primorial)
    
    -- Basic statistics components (store components, not averages)
    count_points BIGINT NOT NULL,           -- number of n values in interval
    
    -- For each alpha channel, store:
    -- Lambda statistics
    sum_lambda_min DOUBLE,                  -- sum of min lambda values
    sum_lambda_max DOUBLE,                  -- sum of max lambda values
    sum_lambda_avg DOUBLE,                  -- sum of avg lambda values
    sum_sq_lambda_min DOUBLE,               -- sum of squares for variance
    sum_sq_lambda_max DOUBLE,
    sum_sq_lambda_avg DOUBLE,
    
    -- Pointwise extremas across all alphas (per n)
    pointwise_min_lambda DOUBLE,            -- minimum lambda across all alphas for this interval
    pointwise_max_lambda DOUBLE,             -- maximum lambda across all alphas for this interval
    pointwise_min_n BIGINT,                 -- n value where pointwise_min occurred
    pointwise_max_n BIGINT,                 -- n value where pointwise_max occurred
    pointwise_min_alpha DOUBLE,            -- alpha where pointwise_min occurred
    pointwise_max_alpha DOUBLE,             -- alpha where pointwise_max occurred
    
    -- Alpha at extrema statistics
    sum_alpha_at_min DOUBLE,                -- sum of alpha values at min extrema
    sum_alpha_at_max DOUBLE,                -- sum of alpha values at max extrema
    count_min_extrema BIGINT,               -- number of times min extrema occurred
    count_max_extrema BIGINT,               -- number of times max extrema occurred
    
    -- First/last values for continuity checks
    first_lambda_min DOUBLE,
    first_lambda_max DOUBLE,
    first_lambda_avg DOUBLE,
    last_lambda_min DOUBLE,
    last_lambda_max DOUBLE,
    last_lambda_avg DOUBLE,
    
    -- Metadata
    size_class TEXT,                        -- SMALL, MEDIUM, LARGE, XPRIM
    version TEXT NOT NULL,                  -- v0.1.5, v0.2.0
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes for fast range queries and rebinning
CREATE INDEX idx_n_range ON lambda_intervals(n_start, n_end);
CREATE INDEX idx_size_class ON lambda_intervals(size_class, version);
CREATE INDEX idx_bin_primorial ON lambda_intervals(bin_primorial_base, bin_index, is_partial);
CREATE INDEX idx_analysis_primorial ON lambda_intervals(analysis_primorial_base, n_start, n_end);
-- Index for rebinning: find all bins that overlap a target range
CREATE INDEX idx_rebin_range ON lambda_intervals(n_start, n_end, bin_primorial_base, analysis_primorial_base);

-- Table for storing histogram bins (optional, for more detailed distribution)
CREATE TABLE lambda_histograms (
    interval_id BIGINT NOT NULL,
    bin_min DOUBLE NOT NULL,
    bin_max DOUBLE NOT NULL,
    count BIGINT NOT NULL,
    FOREIGN KEY (interval_id) REFERENCES lambda_intervals(interval_id),
    PRIMARY KEY (interval_id, bin_min)
);

-- Helper view: computed statistics (derived from components)
CREATE VIEW lambda_intervals_stats AS
SELECT 
    *,
    -- Computed averages
    sum_lambda_min / NULLIF(count_points, 0) AS avg_lambda_min,
    sum_lambda_max / NULLIF(count_points, 0) AS avg_lambda_max,
    sum_lambda_avg / NULLIF(count_points, 0) AS avg_lambda_avg,
    
    -- Computed standard deviations
    SQRT((sum_sq_lambda_min - (sum_lambda_min * sum_lambda_min / NULLIF(count_points, 0))) / NULLIF(count_points - 1, 0)) AS stddev_lambda_min,
    SQRT((sum_sq_lambda_max - (sum_lambda_max * sum_lambda_max / NULLIF(count_points, 0))) / NULLIF(count_points - 1, 0)) AS stddev_lambda_max,
    
    -- Average alpha at extremas
    sum_alpha_at_min / NULLIF(count_min_extrema, 0) AS avg_alpha_at_min,
    sum_alpha_at_max / NULLIF(count_max_extrema, 0) AS avg_alpha_at_max,
    
    -- Range
    pointwise_max_lambda - pointwise_min_lambda AS lambda_range
    
FROM lambda_intervals;

-- Example queries:

-- Find outliers: intervals with unusually high/low lambda
-- SELECT * FROM lambda_intervals_stats
-- WHERE pointwise_min_lambda < (
--     SELECT AVG(pointwise_min_lambda) - 3 * STDDEV(pointwise_min_lambda) 
--     FROM lambda_intervals_stats
-- );

-- Find intervals where alpha at extrema is unusual
-- SELECT * FROM lambda_intervals_stats
-- WHERE avg_alpha_at_min < 0.001 OR avg_alpha_at_max > 0.99;

-- Find intervals near ∏(p-2) boundaries (example: 23# = 223092870)
-- SELECT * FROM lambda_intervals_stats
-- WHERE n_start <= 223092870 AND n_end >= 223092870;

-- Rebin primorial-based bins to decade-based bins
-- (target bins must be larger than source bins)
-- Example: rebin bins (using bin_primorial_base, e.g., sqrt(23#/2)) to decade bins
-- The bin_primorial_value is typically O(√n) while analysis_primorial_value is O(n)
-- WITH target_decades AS (
--     SELECT 
--         FLOOR(n / 1000000.0) AS decade_millions,
--         (FLOOR(n / 1000000.0) * 1000000) AS decade_start,
--         ((FLOOR(n / 1000000.0) + 1) * 1000000) AS decade_end
--     FROM generate_series(1000000, 500000000, 1000000) AS n
-- )
-- SELECT 
--     decade_millions,
--     decade_start,
--     decade_end,
--     -- Aggregate from overlapping primorial bins (weighted by completeness)
--     SUM(sum_lambda_min * bin_completeness) / SUM(count_points * bin_completeness) AS rebinned_avg_lambda_min,
--     SUM(count_points * bin_completeness) AS rebinned_count
-- FROM lambda_intervals li
-- JOIN target_decades td ON li.n_start < td.decade_end AND li.n_end > td.decade_start
-- GROUP BY decade_millions, decade_start, decade_end
-- ORDER BY decade_start;

-- Compare adjacent intervals for discontinuities
-- SELECT 
--     a.interval_id, a.n_end, a.avg_lambda_min,
--     b.interval_id, b.n_start, b.avg_lambda_min,
--     ABS(a.avg_lambda_min - b.avg_lambda_min) AS discontinuity
-- FROM lambda_intervals_stats a
-- JOIN lambda_intervals_stats b ON a.n_end = b.n_start - 1
-- WHERE ABS(a.avg_lambda_min - b.avg_lambda_min) > 0.5
-- ORDER BY discontinuity DESC;

