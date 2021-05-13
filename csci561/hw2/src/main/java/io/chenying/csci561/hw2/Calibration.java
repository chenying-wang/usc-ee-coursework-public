package io.chenying.csci561.hw2;

import java.io.IOException;
import java.io.Writer;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Scanner;

public enum Calibration {

    INSTANCE;

    private final static int CALIBRATION_LENGTH = 7;

    private final static String TARGET_COMMAND = "java calibrate.java run";

    private final static Path TIME_FILE = Paths.get("time.txt");
    private final static Path DEFAULT_INPUT_FILE = Paths.get("input.txt");
    private final static Path DEFAULT_OUTPUT_FILE = Paths.get("output.txt");
    private final static Charset DEFAULT_CHARSET = StandardCharsets.UTF_8;

    private static class CheckersCalibrationEnvironment {

        private final static int MAX_NUM_ROUND = 50;

        private final int depth;
        private final Solution.CheckersBoard board;
        private Solution.CheckersBoard.Color color;
        private int countdown;
        private final Map<String, Integer> countMap;

        public CheckersCalibrationEnvironment(final int depth) {
            this.depth = depth;
            this.board = new Solution.CheckersBoard();
            for (int i = 0; i < 3; ++i) {
                for (int j = (i + 1) % 2; j < Solution.CheckersBoard.SIZE; j += 2) {
                    board.setPiece(Solution.CheckersBoard.Piece.BLACK_PAWN, Solution.CheckersBoard.Point.getPoint(j, i));
                }
            }
            for (int i = Solution.CheckersBoard.SIZE - 3; i < Solution.CheckersBoard.SIZE; ++i) {
                for (int j = (i + 1) % 2; j < Solution.CheckersBoard.SIZE; j += 2) {
                    board.setPiece(Solution.CheckersBoard.Piece.WHITE_PAWN, Solution.CheckersBoard.Point.getPoint(j, i));
                }
            }
            this.color = Solution.CheckersBoard.Color.BLACK;
            this.countdown = 50;
            this.countMap = new HashMap<>();
        }

        public float calibrate() {
            final List<Float> times = new ArrayList<>(CheckersCalibrationEnvironment.MAX_NUM_ROUND);
            for (int i = 0; i < CheckersCalibrationEnvironment.MAX_NUM_ROUND; ++i) {
                boolean cont = this.next(times);
                if (!cont) {
                    break;
                }
            }
            try {
                Files.delete(Calibration.DEFAULT_INPUT_FILE);
            } catch (Exception e) {}

            float sum = 0f;
            for (final float time : times) {
                sum += time;
            }

            final float avgTime = sum / times.size();
            return avgTime;
        }

        private boolean next(final List<Float> times) {
            final String boardStr = this.writeBoard(Calibration.DEFAULT_INPUT_FILE, Calibration.DEFAULT_CHARSET);
            this.countMap.put(boardStr, this.countMap.getOrDefault(boardStr, 0) + 1);
            if (this.countMap.get(boardStr) == 3) {
                return false;
            }

            Solution.CheckersBoard.Action action  = this.run(times);
            try {
                Files.delete(Calibration.TIME_FILE);
                Files.delete(Calibration.DEFAULT_OUTPUT_FILE);
            } catch (Exception e) {}

            if (action == null || action.type == Solution.CheckersBoard.Action.Type.EMPTY) {
                return false;
            } else if (action.type == Solution.CheckersBoard.Action.Type.JUMP ||
                    this.color.touchdown(action.getTarget().y)) {
                this.countdown = 50;
            } else {
                if (--this.countdown == 0) {
                    return false;
                }
            }

            this.board.doAction(action);
            this.color = this.color.next();

            boolean cont = false;
            for (final Solution.CheckersBoard.Piece piece : Solution.CheckersBoard.Piece.getPieces(this.color)) {
                if (!this.board.getPositionsByPiece(piece).isEmpty()) {
                    cont = true;
                    break;
                }
            }
            return cont;
        }

        private Solution.CheckersBoard.Action run(final List<Float> times) {
            Process process = null;
            try {
                process = Runtime.getRuntime().exec(new String[]{"/bin/bash", "-c",
                        "{ time -p " + Calibration.TARGET_COMMAND + " ; } 2> " + Calibration.TIME_FILE.toString()});
            } catch (Exception e) {}
            final Solution.CheckersBoard.Action action = Solution.INSTANCE.solve(null,
                    Calibration.DEFAULT_INPUT_FILE, Calibration.DEFAULT_CHARSET);
            try {
                process.waitFor();
            } catch (Exception e) {}
            float cpuTime = this.readTime(Calibration.TIME_FILE, Calibration.DEFAULT_CHARSET);
            if (cpuTime > 0) {
                times.add(cpuTime);
            }
            return action;
        }

        private float readTime(final Path input, final Charset charset) {
            try (Scanner scanner = new Scanner(Files.newBufferedReader(input, charset))) {
                scanner.nextLine();
                float cpuTime = 0f;
                String userLine = scanner.nextLine(), sysLine = scanner.nextLine();
                cpuTime += Float.valueOf(userLine.substring(5));
                cpuTime += Float.valueOf(sysLine.substring(4));
                return 1e3f * cpuTime;
            } catch (IOException e) {
                return -1;
            }
        }

        private String writeBoard(final Path output, final Charset charset) {
            StringBuilder boardStr = new StringBuilder(Solution.CheckersBoard.SIZE * Solution.CheckersBoard.SIZE);
            try (Writer writer = Files.newBufferedWriter(output, charset)) {
                writer.append(Solution.CheckersSetup.Type.DEBUG.toString()).append('\n')
                    .append(this.color.toString()).append('\n')
                    .append("3600.0\n")
                    .append(Integer.toString(this.depth)).append('\n');

                for (int i = 0; i < Solution.CheckersBoard.SIZE; ++i) {
                    for (int j = 0; j < Solution.CheckersBoard.SIZE; ++j) {
                        final char symbol = this.board.getPieceByPosition(
                                Solution.CheckersBoard.Point.getPoint(j, i)).symbol;
                        writer.append(symbol);
                        boardStr.append(symbol);
                    }
                    writer.append('\n');
                }
                return boardStr.toString();
            } catch (IOException e) {
                return "";
            }
        }
    }

    public void calibrate(final Path output, final Charset charset) {
        final float[] times = new float[Calibration.CALIBRATION_LENGTH];
        for (int i = 0; i < Calibration.CALIBRATION_LENGTH; ++i) {
            final int depth = 2 * i + 1;
            final Calibration.CheckersCalibrationEnvironment env =
                    new Calibration.CheckersCalibrationEnvironment(depth);
            times[i] = Math.max(env.calibrate(), i > 0 ? times[i - 1] : 0);
        }
        this.write(output, times, charset);
    }

    private void write(final Path output, final float[] times, final Charset charset) {
        try (Writer writer = Files.newBufferedWriter(output, charset)) {
            for (final float time : times) {
                writer.append(Float.toString(time)).append('\n');
            }
        } catch (IOException e) {
            return;
        }
    }

}
