package io.chenying.csci561.hw2.test;

import java.util.HashMap;
import java.util.Map;
import java.util.function.Function;

import org.junit.jupiter.api.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import io.chenying.csci561.hw2.Solution;

public class ArenaTest {

    private final static Logger logger = LoggerFactory.getLogger(ArenaTest.class);

    @Test
    public void test() {
        final int[] agent = new int[]{30, 30, 2, 5, 5, 5, -2, -4, -5};
        final Function<Integer, Integer> agentDepth = r -> {
            if (r < 3) {
                return 1;
            } else if (r < 8) {
                return 3;
            } else if (r < 12) {
                return 7;
            }
            return 9;
        };
        final int[] baseline = new int[]{1, 2, 0, 0, 0, 0, 0, 0, 0};
        final Function<Integer, Integer> baselineDepth = r -> 1;

        CheckersEnvironment env = new CheckersEnvironment(
            Map.of(
                Solution.CheckersBoard.Color.BLACK, agentDepth,
                Solution.CheckersBoard.Color.WHITE, baselineDepth
            ),
            Map.of(
                Solution.CheckersBoard.Color.BLACK, agent,
                Solution.CheckersBoard.Color.WHITE, baseline
            )
        );
        env.fight();

        env = new CheckersEnvironment(
            Map.of(
                Solution.CheckersBoard.Color.BLACK, baselineDepth,
                Solution.CheckersBoard.Color.WHITE, agentDepth
            ),
            Map.of(
                Solution.CheckersBoard.Color.BLACK, baseline,
                Solution.CheckersBoard.Color.WHITE, agent
            )
        );
        env.fight();
    }

    private static class CheckersEnvironment {

        private final static int MAX_NUM_ROUND = 200;

        private final Solution.CheckersBoard board;
        private Solution.CheckersBoard.Color color;
        private int countdown;
        private final Map<String, Integer> countMap;

        private final Map<Solution.CheckersBoard.Color, Function<Integer, Integer>> depthMap;
        private final Map<Solution.CheckersBoard.Color, int[]> weightMap;

        public CheckersEnvironment(final Map<Solution.CheckersBoard.Color, Function<Integer, Integer>> depthMap,
                final Map<Solution.CheckersBoard.Color, int[]> weightMap) {
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
            this.depthMap = depthMap;
            this.weightMap = weightMap;
        }

        public void fight() {
            for (int i = 0; i < CheckersEnvironment.MAX_NUM_ROUND; ++i) {
                logger.info("Round {} [{}]:", i, this.color);
                boolean cont = this.next(i / 2);
                logger.info("================");
                if (!cont) {
                    break;
                }
            }
        }

        private boolean next(final int round) {
            final String boardStr = this.writeBoard();
            this.countMap.put(boardStr, this.countMap.getOrDefault(boardStr, 0) + 1);
            if (this.countMap.get(boardStr) == 3) {
                logger.warn("==== DRAW THREE IDENTICAL {} ====", round);
                return false;
            }

            final Solution.CheckersBoard.Action action  = this.run(this.color,
                    this.depthMap.get(this.color).apply(round), this.weightMap.get(this.color));
            StringBuilder actionStr = new StringBuilder();
            action.appendTo(actionStr, '\t');
            logger.info(actionStr.toString());

            if (action == null || action.type == Solution.CheckersBoard.Action.Type.EMPTY) {
                logger.warn("==== INVALID MOVE {} WIN {} ====", this.color.next(), round);
                return false;
            } else if (action.type == Solution.CheckersBoard.Action.Type.JUMP ||
                    this.color.touchdown(action.getTarget().y)) {
                this.countdown = 50;
            } else {
                if (--this.countdown == 0) {
                    logger.warn("==== DRAW FIFTY COUNTDOWN {} ====", round);
                    return false;
                }
            }

            this.board.doAction(action);
            this.color = this.color.next();

            if (this.board.getPositionsByPiece(Solution.CheckersBoard.Piece.BLACK_PAWN).isEmpty() &&
                    this.board.getPositionsByPiece(Solution.CheckersBoard.Piece.BLACK_KING).isEmpty()) {
                logger.warn("==== WHITE WIN {} ====", round);
                return false;
            } else if (this.board.getPositionsByPiece(Solution.CheckersBoard.Piece.WHITE_PAWN).isEmpty() &&
                    this.board.getPositionsByPiece(Solution.CheckersBoard.Piece.WHITE_KING).isEmpty()) {
                logger.warn("==== BLACK WIN {} ====", round);
                return false;
            }
            return true;
        }

        private Solution.CheckersBoard.Action run(final Solution.CheckersBoard.Color color, final int depth, final int[] weight) {
            final Solution.CheckersSetup setup = new Solution.CheckersSetup("DEBUG", color.toString(),
                    Float.POSITIVE_INFINITY, depth, weight, this.board);
            final Solution.CheckersAgent agent = setup.createAgent();
            return agent.next();
        }

        private String writeBoard() {
            StringBuilder boardStr = new StringBuilder(Solution.CheckersBoard.SIZE * Solution.CheckersBoard.SIZE);
            for (int i = 0; i < Solution.CheckersBoard.SIZE; ++i) {
                StringBuilder line = new StringBuilder(Solution.CheckersBoard.SIZE);
                for (int j = 0; j < Solution.CheckersBoard.SIZE; ++j) {
                    final char symbol = this.board.getPieceByPosition(
                            Solution.CheckersBoard.Point.getPoint(j, i)).symbol;
                    line.append(symbol);
                }
                boardStr.append(line.toString());
                logger.info(line.toString());
            }
            logger.info("----------------");
            return boardStr.toString();
        }

    }

}
