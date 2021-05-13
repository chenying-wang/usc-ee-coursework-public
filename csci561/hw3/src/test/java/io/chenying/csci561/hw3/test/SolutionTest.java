package io.chenying.csci561.hw3.test;

import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Scanner;
import java.util.Set;

import org.junit.jupiter.api.Test;
import org.opentest4j.AssertionFailedError;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import io.chenying.csci561.hw3.Solution;

public class SolutionTest {

    private final static Logger logger = LoggerFactory.getLogger(SolutionTest.class);

    private final static String DEFAULT_INPUT_FILE = "input.txt";
    private final static String DEFAULT_OUTPUT_FILE = "output.txt";
    private final static String DEFAULT_OUTPUT_FORMAT = "/tmp/csci561_output_%d.txt";
    private final static Charset DEFAULT_CHARSET = StandardCharsets.UTF_8;

    private final static Path CASES_PATH = Paths.get("test-cases");
    private final static Path LARGE_CASES_PATH = Paths.get("test-cases-large");
    private final static Path GRADING_CASES_PATH = Paths.get("test-cases-grading");

    private final static String FALSE_OUTPUT = "FALSE";
    private final static String TRUE_OUTPUT = "TRUE";

    @Test
    public void testOneCase() {
        final String testCase = "case01";
        this.testOneCase(SolutionTest.CASES_PATH.resolve(testCase), SolutionTest.DEFAULT_CHARSET);
    }

    @Test
    public void testAllCases() {
        this.testAllCases(SolutionTest.CASES_PATH, SolutionTest.DEFAULT_CHARSET);
    }

    @Test
    public void testAllLargeCases() {
        this.testAllCases(SolutionTest.LARGE_CASES_PATH, SolutionTest.DEFAULT_CHARSET);
    }

    @Test
    public void testAllGradingCases() {
        this.testAllCases(SolutionTest.GRADING_CASES_PATH, SolutionTest.DEFAULT_CHARSET);
    }

    private void testAllCases(final Path casesPath, final Charset charset) {
        int[] count = new int[3];
        final Set<String> failedCases = new HashSet<>();
        try {
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
        } catch (Exception e) {
            logger.error("Failed to open {}", casesPath, e);
        }

        logger.warn("#### SUMMARY ####");
        for (final String failedCase : failedCases) {
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

    private void assertCase(final Path expected, final Path actual, final Path input, final Charset charset) {
        final List<Boolean> expectedAnswers = this.readOutput(expected, charset);
        final List<Boolean> actualAnswers = this.readOutput(actual, charset);
        if (expectedAnswers.size() != actualAnswers.size()) {
            throw new AssertionFailedError(new StringBuilder("Actual size of answers ")
                    .append(actualAnswers.size())
                    .append(" does not match with expected size of answers ")
                    .append(expectedAnswers.size()).toString());
        }

        int failedCount = 0;
        for (int i = 0; i < expectedAnswers.size(); ++i) {
            Boolean expectedAnswer = expectedAnswers.get(i);
            Boolean actualAnswer = actualAnswers.get(i);
            boolean failed = expectedAnswer != actualAnswer;
            if (failed) {
                logger.error("Answer #{}, Expected: {}, Actual: {}", i + 1, expectedAnswer, actualAnswer);
                ++failedCount;
            } else {
                logger.debug("Answer #{}, Expected: {}, Actual: {}", i + 1, expectedAnswer, actualAnswer);
            }
        }
        if (failedCount > 0) {
            throw new AssertionFailedError("Assertion failed in " + failedCount + " answers");
        }
    }

    private List<Boolean> readOutput(final Path output, final Charset charset) {
        final List<Boolean> result = new ArrayList<>();
        try (Scanner scanner = new Scanner(Files.newBufferedReader(output, charset))) {
            while (scanner.hasNextLine()) {
                String line = scanner.nextLine();
                if (SolutionTest.TRUE_OUTPUT.equals(line)) {
                    result.add(true);
                } else if (SolutionTest.FALSE_OUTPUT.equals(line)) {
                    result.add(false);
                } else {
                    result.add(null);
                }
            }
        } catch (Exception e) {
            logger.error("Failed to read {}", output, e.getMessage());
        }
        return result;
    }

}
