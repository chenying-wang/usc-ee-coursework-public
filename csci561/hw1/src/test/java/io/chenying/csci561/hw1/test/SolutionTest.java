package io.chenying.csci561.hw1.test;

import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Scanner;
import java.util.Set;

import org.junit.jupiter.api.Test;
import org.opentest4j.AssertionFailedError;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import io.chenying.csci561.hw1.Solution;
import io.chenying.csci561.hw1.Solution.SearchAlgorithm;

public class SolutionTest {

    private final static Logger logger = LoggerFactory.getLogger(SolutionTest.class);

    private final static String DEFAULT_INPUT_FILE = "input.txt";
    private final static String DEFAULT_OUTPUT_FILE = "output.txt";
    private final static String DEFAULT_OUTPUT_FORMAT = "/tmp/csci561_output_%d.txt";
    private final static Charset DEFAULT_CHARSET = StandardCharsets.UTF_8;

    private final static Path CASES_PATH = Paths.get("test-cases");
    private final static Path LARGE_CASES_PATH = Paths.get("test-cases-large");
    private final static Path GRADING_CASES_PATH = Paths.get("test-cases-grading");

    private final static String FAIL = "FAIL";

    @Test
    public void testOneCase() {
        final String testCase = "case01";
        this.testOneCase(SolutionTest.CASES_PATH.resolve(testCase), SolutionTest.DEFAULT_CHARSET);
    }

    @Test
    public void testAllCases() throws IOException {
        this.testAllCases(SolutionTest.CASES_PATH, SolutionTest.DEFAULT_CHARSET);
    }

    @Test
    public void testAllLargeCases() throws IOException {
        this.testAllCases(SolutionTest.LARGE_CASES_PATH, SolutionTest.DEFAULT_CHARSET);
    }

    @Test
    public void testAllGradingCases() throws IOException {
        this.testAllCases(SolutionTest.GRADING_CASES_PATH, SolutionTest.DEFAULT_CHARSET);
    }

    private void testAllCases(final Path casesPath, final Charset charset) throws IOException {
        int[] count = new int[3];
        final Set<String> failedCases = new HashSet<>();
        Files.list(casesPath)
                .filter(Files::isDirectory)
                .sorted()
                .forEach(p -> {
                    final int status = this.testOneCase(p, charset);
                    ++count[status];
                    if (status == 1) {
                        failedCases.add(p.getFileName().toString());
                    }
                    logger.info("");
                });

        logger.warn("#### SUMMARY ####");
        for (String failedCase : failedCases) {
            logger.error("{} FAILED", failedCase);
        }
        logger.warn("{}/{} CASES PASSED and {} CASES SKIPPED", count[0], count[0] + count[1], count[2]);
        logger.warn("#### ####### ####");
    }

    private int testOneCase(final Path casePath, final Charset charset) {
        logger.info("## {} START ##", casePath.getFileName());
        try {
            long elapsed = this.testCase(casePath, charset);
            logger.info("## {} PASSED with {}ms ##", casePath.getFileName(), elapsed / 1e6);
            return 0;
        } catch (IllegalArgumentException e) {
            logger.warn("## {} SKIPPED as {} ##", casePath.getFileName(), e.getMessage());
            return 2;
        } catch (AssertionError e) {
            logger.error("## {} FAILED as {} ##", casePath.getFileName(), e.getMessage());
            return 1;
        } catch (Exception e) {
            logger.error("## {} FAILED as {} ##", casePath.getFileName(), e.getMessage(), e);
        }
        return 1;
    }

    private long testCase(final Path casePath, final Charset charset) throws IllegalArgumentException, RuntimeException {
        if (casePath == null) {
            logger.warn("Invalid case path {}", casePath);
            throw new IllegalArgumentException("Invalid case path " + casePath);
        }

        final Path input = casePath.resolve(SolutionTest.DEFAULT_INPUT_FILE);
        final Path expectedOutput = casePath.resolve(SolutionTest.DEFAULT_OUTPUT_FILE);
        final Path actualOutput =  Paths.get(String.format(DEFAULT_OUTPUT_FORMAT, System.nanoTime()));
        if (!(Files.exists(input) && Files.exists(expectedOutput))) {
            logger.warn("Invalid case path {}", casePath);
            throw new IllegalArgumentException("Invalid case path " + casePath);
        }
        Exception ex = null;
        long start = 0, end = Long.MAX_VALUE;
        try {
            start = System.nanoTime();
            Solution.INSTANCE.solve(actualOutput, input, charset);
            end = System.nanoTime();
        } catch (Exception e) {
            end = System.nanoTime();
            ex = e;
        }

        if (ex != null) {
            try {
                Files.delete(actualOutput);
            } catch (IOException e) {
                logger.error("File deletion error {}", e.getMessage());
            }
            throw new RuntimeException("Exception caught from Solution", ex);
        }
        try {
            this.assertCase(expectedOutput, actualOutput, input, charset);
        } finally {
            try {
                Files.delete(actualOutput);
            } catch (IOException e) {
                logger.error("File deletion error {}", e.getMessage());
            }
        }
        return end - start;
    }

    private static class GameMap {
        final String algorithm;
        final int width;
        final int height;
        final int sourceX;
        final int sourceY;
        final int maxRockHeight;
        final int numTargets;
        final int[][] targets;
        final int[][] map;

        public GameMap(final Path input, final Charset charset) throws IOException {
            final Scanner scanner = new Scanner(input, charset);
            this.algorithm = scanner.nextLine();
            this.width = scanner.nextInt();
            this.height = scanner.nextInt();
            this.sourceX = scanner.nextInt();
            this.sourceY = scanner.nextInt();
            this.maxRockHeight = scanner.nextInt();
            this.numTargets = scanner.nextInt();
            this.targets = new int[numTargets][2];
            for (int i = 0; i < numTargets; ++i) {
                this.targets[i][0] = scanner.nextInt();
                this.targets[i][1] = scanner.nextInt();
            }
            this.map = new int[height][width];
            for (int i = 0; i < height; ++i) {
                for (int j = 0; j < width; ++j) {
                    this.map[i][j] = scanner.nextInt();
                }
            }
            scanner.close();
        }

        public long getCost(final int[][] path) {
            if (path == null) {
                return -1;
            } else if (path.length < 2 || path[0][0] != this.sourceX || path[0][1] != this.sourceY) {
                logger.warn("Invalid path encountered");
                return Long.MAX_VALUE;
            }
            long cost = 0;
            logger.trace("--------");
            for (int i = 1; i < path.length; ++i) {
                int dx = Math.abs(path[i][0] - path[i - 1][0]);
                int dy = Math.abs(path[i][1] - path[i - 1][1]);
                int dh = this.getHeightCost(path[i - 1], path[i]);
                logger.trace("{}: [{},{}({}) -> {},{}({})]: {{}, {}, {}}", i - 1,
                    path[i - 1][0], path[i - 1][1], this.map[path[i - 1][1]][path[i - 1][0]],
                    path[i][0], path[i][1], this.map[path[i][1]][path[i][0]],
                    path[i][0] - path[i - 1][0], path[i][1] - path[i - 1][1], dh);
                if (dx > 1 || dy > 1 || dh > this.maxRockHeight) {
                    logger.error("Illegal move from {},{}({}) to {},{}({}) with max diff {}",
                        path[i - 1][0], path[i - 1][1], this.map[path[i - 1][1]][path[i - 1][0]],
                        path[i][0], path[i][1], this.map[path[i][1]][path[i][0]], this.maxRockHeight);
                    return Long.MAX_VALUE;
                }
                cost += this.getMoveCost(dx, dy, dh, Math.max(this.map[path[i][1]][path[i][0]], 0));
            }
            return cost;
        }

        private long getMoveCost(final int dx, final int dy, final int dh, final int m) {
            if ("BFS".equals(this.algorithm)) {
                return 1;
            }
            long cost = dx + dy == 1 ? SearchAlgorithm.STRAIGHT_MOVE_COST : SearchAlgorithm.DIAGONAL_MOVE_COST;
            if ("UCS".equals(this.algorithm)) {
                return cost;
            }
            cost += dh +  m;
            return cost;
        }

        private int getHeightCost(final int[] from, final int[] to) {
            return Math.abs(Math.min(map[to[1]][to[0]], 0) - Math.min(map[from[1]][from[0]], 0));
        }

    }

    private void assertCase(final Path expected, final Path actual, final Path input, final Charset charset) {
        final GameMap map = this.readInput(input, charset);
        final List<int[][]> expectedPaths = this.readOutput(expected, charset);
        final List<int[][]> actualPaths = this.readOutput(actual, charset);
        if (expectedPaths.size() != actualPaths.size()) {
            throw new AssertionFailedError("Actual size of paths " + actualPaths.size() + " does not match with expected size of paths " + expectedPaths.size());
        }
        int failedCount = 0;
        for (int i = 0; i < expectedPaths.size(); ++i) {
            int[][] expectedPath = expectedPaths.get(i);
            int[][] actualPath = actualPaths.get(i);
            int expectedLength = expectedPath != null ? expectedPath.length : -1;
            int actualLength = actualPath != null ? actualPath.length : -1;
            long expectedCost = map.getCost(expectedPath);
            long actualCost = map.getCost(actualPath);

            logger.debug("Algo: {}, Path #{} [{},{} -> {},{}]: {Expected cost: {} with length: {}, Actual cost: {} with length: {}}",
                map.algorithm, i,
                map.sourceX, map.sourceY, map.targets[i][0], map.targets[i][1],
                expectedCost == -1 ? "Inf" : expectedCost, expectedLength == -1 ? "None" : expectedLength - 1,
                actualCost == -1 ? "Inf" : actualCost, actualLength == -1 ? "None" : actualLength - 1);

            if (expectedLength != -1 && actualLength == -1) {
                logger.error("Algo: {}, Path #{} [{},{} -> {},{}]: {Expected path: Found, Actual path: Not found}",
                    map.algorithm, i,
                    map.sourceX, map.sourceY, map.targets[i][0], map.targets[i][1]);
                    ++failedCount;
            } else if (expectedLength == -1 && actualLength != -1) {
                logger.warn("Algo: {}, Path #{} [{},{} -> {},{}]: {Expected path: Not found, Actual path: Found}",
                    map.algorithm, i,
                    map.sourceX, map.sourceY, map.targets[i][0], map.targets[i][1]);
            } else if (actualCost > expectedCost) {
                logger.error("Algo: {}, Path #{} [{},{} -> {},{}]: {Actual cost {} is greater than expected cost {}}",
                    map.algorithm, i,
                    map.sourceX, map.sourceY, map.targets[i][0], map.targets[i][1],
                    actualCost, expectedCost);
                    ++failedCount;
            } else if (actualCost < expectedCost) {
                logger.warn("Algo: {}, Path #{} [{},{} -> {},{}]: {Actual cost {} is less than expected cost {}}",
                    map.algorithm, i,
                    map.sourceX, map.sourceY, map.targets[i][0], map.targets[i][1],
                    actualCost, expectedCost);
            } else if (expectedLength != actualLength) {
                logger.warn("Algo: {}, Path #{} [{},{} -> {},{}]: {Actual path length {} does not equal expected path length {}}",
                    map.algorithm, i,
                    map.sourceX, map.sourceY, map.targets[i][0], map.targets[i][1],
                    actualLength - 1, expectedLength - 1);
            }
        }
        if (failedCount > 0) {
            throw new AssertionFailedError("Assertion failed in " + failedCount + " paths");
        }
    }

    private GameMap readInput(final Path input, final Charset charset) {
        try {
            return new GameMap(input, charset);
        } catch (IOException e) {
            logger.error("Failed to read {}", input, e.getMessage());
        }
        return null;
    }

    private List<int[][]> readOutput(final Path output, final Charset charset) {
        List<int[][]> paths = new LinkedList<>();
        try (Scanner scanner = new Scanner(output, charset)) {
            while (scanner.hasNextLine()) {
                String line = scanner.nextLine();
                if (SolutionTest.FAIL.equals(line)) {
                    paths.add(null);
                    continue;
                }
                String[] points = line.split(" ");
                int[][] path = new int[points.length][2];
                for (int i = 0; i < path.length; ++i) {
                    String[] point = points[i].split(",");
                    path[i][0] = Integer.parseInt(point[0]);
                    path[i][1] = Integer.parseInt(point[1]);
                }
                paths.add(path);
            }
            return paths;
        } catch (IOException e) {
            logger.error("Failed to write {}", output, e.getMessage());
        }
        return null;
    }

}
