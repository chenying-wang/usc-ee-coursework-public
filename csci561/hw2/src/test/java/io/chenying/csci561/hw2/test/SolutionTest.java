package io.chenying.csci561.hw2.test;

import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashSet;
import java.util.Set;

import org.junit.jupiter.api.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import io.chenying.csci561.hw2.Calibration;
import io.chenying.csci561.hw2.Solution;

public class SolutionTest {

    private final static Logger logger = LoggerFactory.getLogger(SolutionTest.class);

    private final static String DEFAULT_INPUT_FILE = "input.txt";
    private final static String DEFAULT_OUTPUT_FILE = "output.txt";
    private final static String DEFAULT_OUTPUT_FORMAT = "/tmp/csci561_output_%d.txt";
    private final static Charset DEFAULT_CHARSET = StandardCharsets.UTF_8;

    private final static Path CASES_PATH = Paths.get("test-cases");

    private final static Path CALIBRATION_FILE = Paths.get("calibration.txt");


    @Test
    public void testTwoAgent() {

    }

    @Test
    public void testCalibration() {
        Calibration.INSTANCE.calibrate(SolutionTest.CALIBRATION_FILE, SolutionTest.DEFAULT_CHARSET);
    }

    @Test
    public void testOneCase() {
        final String testCase = "case01";
        this.testOneCase(SolutionTest.CASES_PATH.resolve(testCase), SolutionTest.DEFAULT_CHARSET);
    }

    @Test
    public void testAllCases() throws IOException {
        this.testAllCases(SolutionTest.CASES_PATH, SolutionTest.DEFAULT_CHARSET);
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
            Files.delete(actualOutput);
        } catch (IOException e) {
            logger.error("File deletion error {}", e.getMessage());
        }
        return end - start;
    }

}
