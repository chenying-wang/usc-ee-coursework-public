package io.chenying.csci561.hw2;

import java.io.IOException;
import java.io.Writer;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Deque;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.PriorityQueue;
import java.util.Queue;
import java.util.Scanner;
import java.util.Set;
import java.util.function.Function;

public enum Solution {

    INSTANCE;

    private final static int DEFAULT_SEARCH_DEPTH = 9;

    public static class CheckersBoard {

        public final static int SIZE = 8;

        public final static int NUM_POINTS = CheckersBoard.SIZE * CheckersBoard.SIZE / 2;
        public final static int MAX_NUM_MOVES = CheckersBoard.NUM_POINTS - 1;

        public static enum Color {
            BLACK(1, CheckersBoard.SIZE - 1),
            WHITE(-1, 0);

            public int forward;
            public int backward;
            public int touchdown;

            Color(final int forward, final int touchdown) {
                this.forward = forward;
                this.backward = -forward;
                this.touchdown = touchdown;
            }

            public Color next() {
                return this == Color.BLACK ? Color.WHITE : Color.BLACK;
            }

            public boolean touchdown(final int y) {
                return y == this.touchdown;
            }
        }

        public static enum Piece {
            EMPTY('.'),
            OUT_OF_BOUNDARY('\0'),
            BLACK_KING('B', CheckersBoard.Color.BLACK, new int[][]{
                {-1, CheckersBoard.Color.BLACK.forward},
                {1, CheckersBoard.Color.BLACK.forward},
                {-1, CheckersBoard.Color.BLACK.backward},
                {1, CheckersBoard.Color.BLACK.backward}
            }),
            BLACK_PAWN('b', CheckersBoard.Color.BLACK, new int[][]{
                {-1, CheckersBoard.Color.BLACK.forward},
                {1, CheckersBoard.Color.BLACK.forward}
            }),
            WHITE_KING('W', CheckersBoard.Color.WHITE, new int[][]{
                {-1, CheckersBoard.Color.WHITE.forward},
                {1, CheckersBoard.Color.WHITE.forward},
                {-1, CheckersBoard.Color.WHITE.backward},
                {1, CheckersBoard.Color.WHITE.backward}
            }),
            WHITE_PAWN('w', CheckersBoard.Color.WHITE, new int[][]{
                {-1, CheckersBoard.Color.WHITE.forward},
                {1, CheckersBoard.Color.WHITE.forward}
            });

            private static Map<Character, Piece> symbolMap;
            private static Map<CheckersBoard.Color, List<Piece>> colorMap;
            private static Map<Piece, Piece> crownMap;
            private static Map<Piece, Piece> uncrownMap;

            static {
                Piece.symbolMap = new HashMap<>(Piece.values().length);
                for (Piece piece : Piece.values()) {
                    Piece.symbolMap.put(piece.symbol, piece);
                }
                Piece.colorMap = new HashMap<>(CheckersBoard.Color.values().length);
                for (Piece piece : Piece.values()) {
                    if (piece.color != null) {
                        Piece.colorMap.putIfAbsent(piece.color, new ArrayList<>());
                        Piece.colorMap.get(piece.color).add(piece);
                    }
                }
                Piece.crownMap = Map.of(
                    Piece.BLACK_PAWN, Piece.BLACK_KING,
                    Piece.WHITE_PAWN, Piece.WHITE_KING
                );
                Piece.uncrownMap = Map.of(
                    Piece.BLACK_KING, Piece.BLACK_PAWN,
                    Piece.WHITE_KING, Piece.WHITE_PAWN
                );
            }

            public char symbol;
            public CheckersBoard.Color color;
            public int[][] moves;

            public static Piece of(final char symbol) {
                return Piece.symbolMap.get(symbol);
            }

            public static List<Piece> getPieces(final CheckersBoard.Color color) {
                return Piece.colorMap.get(color);
            }

            Piece(final char symbol, final CheckersBoard.Color color, final int[][] moves) {
                this.symbol = symbol;
                this.color = color;
                this.moves = moves;
            }

            Piece(final char symbol) {
                this(symbol, null, null);
            }

            public boolean crownable() {
                return Piece.crownMap.containsKey(this);
            }

            public Piece crown() {
                return Piece.crownMap.get(this);
            }

            public Piece uncrown() {
                return Piece.uncrownMap.get(this);
            }
        }

        public static class Point {

            private final static char[] HORIZONTAL_SYMBOLS = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
            private final static char[] VERTICAL_SYMBOLS = {'8', '7', '6', '5', '4', '3', '2', '1'};
            private static Point[][] pool;

            static {
                Point.pool = new Point[CheckersBoard.SIZE][CheckersBoard.SIZE];
            }

            public final int x;
            public final int y;
            private final String symbol;

            final boolean isSafe;
            final boolean isCentral;

            private Point(final int x, final int y) {
                this.x = x;
                this.y = y;
                this.symbol = new StringBuilder(2)
                    .append(Point.HORIZONTAL_SYMBOLS[x])
                    .append(Point.VERTICAL_SYMBOLS[y]).toString();
                this.isSafe = this.x == 0 || this.x == CheckersBoard.SIZE - 1 ||
                        this.y == 0 || this.y == CheckersBoard.SIZE - 1;
                this.isCentral = this.x > 1 && this.x < CheckersBoard.SIZE - 2 &&
                        this.y > 1 && this.y < CheckersBoard.SIZE - 2;
            }

            public static Point getPoint(final int x, final int y) {
                if (x < 0 || x >= CheckersBoard.SIZE || y < 0 || y >= CheckersBoard.SIZE) {
                    return null;
                } else if (Point.pool[y][x] == null) {
                    Point.pool[y][x] = new Point(x, y);
                }
                return Point.pool[y][x];
            }

            @Override
            public int hashCode() {
                return Objects.hash(this.x, this.y);
            }

            @Override
            public boolean equals(Object obj) {
                if (obj instanceof Point) {
                    return ((Point) obj).x == this.x && ((Point) obj).y == this.y;
                }
                return false;
            }

            @Override
            public String toString() {
                return this.symbol;
            }

        }

        public static class Action implements Comparable<Action> {

            private final static Action EMPTY_ACTION = new Action(Action.Type.EMPTY, null, List.of());

            public static enum Type {
                EMPTY('\0'),
                ADJACENT_MOVE('E'),
                JUMP('J');

                private char symbol;

                Type(final char symbol) {
                    this.symbol = symbol;
                }
            }

            public final Action.Type type;
            private final List<CheckersBoard.Point> path;
            private final List<CheckersBoard.Point> capturedPoints;
            private final Map<CheckersBoard.Point, CheckersBoard.Piece> capturedMap;
            private final boolean touchedDown;

            public Action(final Action.Type type, final CheckersBoard.Piece piece,
                    final List<CheckersBoard.Point> path,
                    final List<CheckersBoard.Point> capturedPoints,
                    final Map<CheckersBoard.Point, CheckersBoard.Piece> capturedMap) {
                this.type = type;
                this.path = path;
                this.capturedPoints = capturedPoints;
                this.capturedMap = capturedMap;
                this.touchedDown = type != Action.Type.EMPTY && piece.crownable() &&
                        piece.color.touchdown(this.getTarget().y);
            }

            public Action(final Action.Type type, final CheckersBoard.Piece piece,
                    final List<CheckersBoard.Point> path) {
                this(type, piece, path, List.of(), Map.of());
            }

            public CheckersBoard.Point getSource() {
                return this.path.get(0);
            }

            public CheckersBoard.Point getTarget() {
                return this.path.get(this.path.size() - 1);
            }

            public void appendTo(final Appendable appendable, final char separator) {
                if (this.type == Action.Type.EMPTY) {
                    return;
                }
                final Iterator<CheckersBoard.Point> fromIt = this.path.iterator();
                final Iterator<CheckersBoard.Point> toIt = this.path.iterator();
                try {
                    for (toIt.next(); toIt.hasNext();) {
                        appendable.append(this.type.symbol).append(' ')
                            .append(fromIt.next().toString()).append(' ')
                            .append(toIt.next().toString());
                        if (toIt.hasNext()) {
                            appendable.append(separator);
                        }
                    }
                } catch (IOException e) {
                    return;
                }
            }

            @Override
            public int compareTo(final Action o) {
                if (o == null) {
                    return 1;
                } else if (o.type == Action.Type.EMPTY) {
                    return this.type == Action.Type.EMPTY ? 0 : 1;
                }
                return Integer.compare(o.capturedPoints.size(), this.capturedPoints.size());
            }

            @Override
            public String toString() {
                if (this.path.size() < 2) {
                    return "";
                }
                final StringBuilder strBuilder = new StringBuilder(8 * (this.path.size() - 1));
                this.appendTo(strBuilder, ';');
                return strBuilder.toString();
            }

        }

        private static class PendingJumpAction {

            private Deque<CheckersBoard.Point> path;
            private Deque<CheckersBoard.Point> capturedPoints;
            private Map<CheckersBoard.Point, CheckersBoard.Piece> capturedMap;

            public PendingJumpAction(final CheckersBoard.Point source) {
                this.path = new LinkedList<>();
                this.capturedPoints = new LinkedList<>();
                this.capturedMap = new HashMap<>();
                this.path.offerLast(source);
            }

            public void push(final CheckersBoard.Point point, final CheckersBoard.Point capturedPoint,
                    final CheckersBoard.Piece capturedPiece) {
                this.path.offerLast(point);
                this.capturedPoints.offerLast(capturedPoint);
                this.capturedMap.put(capturedPoint, capturedPiece);
            }

            public void pop() {
                this.path.pollLast();
                this.capturedMap.remove(this.capturedPoints.pollLast());
            }

            public CheckersBoard.Point peek() {
                return this.path.peekLast();
            }

            public CheckersBoard.Point getSource() {
                return this.path.getFirst();
            }

            public boolean visited(final CheckersBoard.Point capturedPoint) {
                return this.capturedPoints.contains(capturedPoint);
            }

            public int size() {
                return this.path.size();
            }

            public CheckersBoard.Action toAction(final CheckersBoard.Piece piece) {
                return new CheckersBoard.Action(CheckersBoard.Action.Type.JUMP, piece,
                        List.copyOf(this.path), List.copyOf(this.capturedPoints), Map.copyOf(this.capturedMap));
            }

        }

        private final CheckersBoard.Piece[][] pieces;
        private final Map<CheckersBoard.Piece, Set<CheckersBoard.Point>> piecePositions;

        public CheckersBoard() {
            this.pieces = new CheckersBoard.Piece[CheckersBoard.SIZE][CheckersBoard.SIZE];
            for (int i = 0; i < CheckersBoard.SIZE; ++i) {
                for (int j = 0; j < CheckersBoard.SIZE; ++j) {
                    this.pieces[i][j] = CheckersBoard.Piece.EMPTY;
                }
            }
            this.piecePositions = new HashMap<>();
            for (Piece piece : CheckersBoard.Piece.values()) {
                if (piece.color != null) {
                    this.piecePositions.put(piece, new HashSet<>(CheckersBoard.NUM_POINTS));
                }
            }
        }

        public CheckersBoard.Piece getPieceByPosition(final CheckersBoard.Point point) {
            if (point == null) {
                return CheckersBoard.Piece.OUT_OF_BOUNDARY;
            }
            return this.pieces[point.y][point.x];
        }

        public Set<CheckersBoard.Point> getPositionsByPiece(final CheckersBoard.Piece piece) {
            return this.piecePositions.get(piece);
        }

        public void setPiece(final CheckersBoard.Piece piece, final CheckersBoard.Point point) {
            if (this.pieces[point.y][point.x].color != null) {
                this.piecePositions.get(this.pieces[point.y][point.x]).remove(point);
            }
            this.pieces[point.y][point.x] = piece;
            if (piece.color != null) {
                this.piecePositions.get(piece).add(point);
            }
        }

        public void doAction(final CheckersBoard.Action action) {
            if (action.type == CheckersBoard.Action.Type.EMPTY) {
                return;
            }

            Piece piece = this.getPieceByPosition(action.getSource());
            if (action.touchedDown) {
                piece = piece.crown();
            }
            this.setPiece(CheckersBoard.Piece.EMPTY, action.getSource());
            this.setPiece(piece, action.getTarget());
            if (action.type == CheckersBoard.Action.Type.ADJACENT_MOVE) {
                return;
            }
            for (final CheckersBoard.Point point : action.capturedPoints) {
                this.setPiece(CheckersBoard.Piece.EMPTY, point);
            }
        }

        public void undoAction(final CheckersBoard.Action action) {
            if (action.type == CheckersBoard.Action.Type.EMPTY) {
                return;
            }

            Piece piece = this.getPieceByPosition(action.getTarget());
            if (action.touchedDown) {
                piece = piece.uncrown();
            }
            this.setPiece(CheckersBoard.Piece.EMPTY, action.getTarget());
            this.setPiece(piece, action.getSource());
            if (action.type == CheckersBoard.Action.Type.ADJACENT_MOVE) {
                return;
            }
            for (final CheckersBoard.Point point : action.capturedPoints) {
                this.setPiece(action.capturedMap.get(point), point);
            }
        }

        public Queue<CheckersBoard.Action> getValidActions(final Queue<CheckersBoard.Action> actions,
                final CheckersBoard.Color color, final int maxNumActions) {
            for (final CheckersBoard.Piece piece : CheckersBoard.Piece.getPieces(color)) {
                for (final CheckersBoard.Point point : this.getPositionsByPiece(piece)) {
                    this.appendJumpActions(actions, point, piece, maxNumActions);
                    if (actions.size() >= maxNumActions) {
                        return actions;
                    }
                }
            }
            if (!actions.isEmpty()) {
                return actions;
            }

            for (final CheckersBoard.Piece piece : CheckersBoard.Piece.getPieces(color)) {
                for (final CheckersBoard.Point point : this.getPositionsByPiece(piece)) {
                    this.appendSimpleActions(actions, point, piece, maxNumActions);
                    if (actions.size() >= maxNumActions) {
                        return actions;
                    }
                }
            }
            return actions;
        }

        private void appendJumpActions(final Queue<CheckersBoard.Action> actions,
                final CheckersBoard.Point source, final CheckersBoard.Piece piece,
                final int maxNumActions) {
            this.appendJumpActions(actions, new CheckersBoard.PendingJumpAction(source), piece, maxNumActions);
        }

        private void appendSimpleActions(final Queue<CheckersBoard.Action> actions,
                final CheckersBoard.Point source, final CheckersBoard.Piece piece, final int maxNumActions) {
            final int x = source.x, y = source.y;
            for (final int[] move : piece.moves) {
                final CheckersBoard.Point nextPoint = CheckersBoard.Point.getPoint(x + move[0], y + move[1]);
                if (this.getPieceByPosition(nextPoint) != CheckersBoard.Piece.EMPTY) {
                    continue;
                }
                actions.offer(new CheckersBoard.Action(CheckersBoard.Action.Type.ADJACENT_MOVE, piece,
                        List.of(source, nextPoint)));
                if (actions.size() >= maxNumActions) {
                    return;
                }
            }
        }

        private void appendJumpActions(final Queue<CheckersBoard.Action> actions,
                final CheckersBoard.PendingJumpAction pendingAction, final CheckersBoard.Piece piece,
                final int maxNumActions) {
            if (actions.size() >= maxNumActions) {
                return;
            }

            boolean canContinue = false;
            for (final int[] move : piece.moves) {
                if (!this.checkJump(pendingAction, piece, move[0], move[1])) {
                    continue;
                }
                canContinue = true;
                this.appendJumpActions(actions, pendingAction, piece, maxNumActions);
                pendingAction.pop();
                if (actions.size() >= maxNumActions) {
                    return;
                }
            }
            if (!canContinue && pendingAction.size() > 1) {
                actions.offer(pendingAction.toAction(piece));
            }
        }

        private boolean checkJump(final CheckersBoard.PendingJumpAction pendingAction, final CheckersBoard.Piece piece,
                final int dx, final int dy) {
            final CheckersBoard.Point point = pendingAction.peek();
            final int nx = point.x + dx, ny = point.y + dy;
            final CheckersBoard.Point nextPoint =  CheckersBoard.Point.getPoint(nx + dx, ny + dy);
            final CheckersBoard.Piece next = this.getPieceByPosition(nextPoint);
            if (next != CheckersBoard.Piece.EMPTY && nextPoint != pendingAction.getSource()) {
                return false;
            }
            final CheckersBoard.Point capturedPoint = CheckersBoard.Point.getPoint(nx, ny);
            if (pendingAction.visited(capturedPoint)) {
                return false;
            }
            final CheckersBoard.Piece capturedPiece = this.getPieceByPosition(capturedPoint);
            if (capturedPiece == CheckersBoard.Piece.EMPTY || capturedPiece.color == piece.color) {
                return false;
            }
            pendingAction.push(nextPoint, capturedPoint, capturedPiece);
            return true;
        }

    }

    public static abstract class CheckersAgent {

        protected final CheckersBoard.Color myColor;
        protected final CheckersBoard board;

        public CheckersAgent(final CheckersBoard.Color myColor, final CheckersBoard board) {
            this.myColor = myColor;
            this.board = board;
        }

        public abstract CheckersBoard.Action next();

        protected Queue<CheckersBoard.Action> expand(final Queue<CheckersBoard.Action> queue,
                final CheckersBoard.Color color, final int maxNumActions) {
            return this.board.getValidActions(queue, color, maxNumActions);
        }

        protected Queue<CheckersBoard.Action> expand(final Queue<CheckersBoard.Action> queue,
                final CheckersBoard.Color color) {
            return this.expand(queue, color, CheckersBoard.MAX_NUM_MOVES);
        }

    }

    private static class SingleMoveCheckersAgent extends CheckersAgent {

        private final static int NUM_MOVES = 1;

        public SingleMoveCheckersAgent(final CheckersBoard.Color myColor, final CheckersBoard board) {
            super(myColor, board);
        }

        @Override
        public CheckersBoard.Action next() {
            final Queue<CheckersBoard.Action> actions = this.expand(new LinkedList<>(), this.myColor, SingleMoveCheckersAgent.NUM_MOVES);
            return actions.peek();
        }

    }

    private static class GameCheckersAgent extends CheckersAgent {

        private final static Path CALIBRATION_FILE = Paths.get("calibration.txt");
        private final static Path STATE_FILE = Paths.get("playdata.txt");
        private final static Charset DEFAULT_CHARSET = StandardCharsets.UTF_8;

        private final static int[] UTILITY_WEIGHT = {30, 30, 2, 5, 5, 5, -2, -4, -5};

        private final static int ESTIMATED_ROUND = 40;

        private final int depth;
        private final int[] utilityWeight;

        public GameCheckersAgent(final CheckersBoard.Color myColor, final CheckersBoard board, final float remainingTime) {
            super(myColor, board);
            final int round = this.readState(GameCheckersAgent.STATE_FILE, GameCheckersAgent.DEFAULT_CHARSET);
            int depth = Solution.DEFAULT_SEARCH_DEPTH;
            if (remainingTime > 500 && round > 2) {
                final float estimatedTime = round < GameCheckersAgent.ESTIMATED_ROUND - 3 ?
                        remainingTime / (GameCheckersAgent.ESTIMATED_ROUND - round) : remainingTime / 3;
                final float[] times = this.readCalibration(
                        GameCheckersAgent.CALIBRATION_FILE, GameCheckersAgent.DEFAULT_CHARSET);
                if (times != null) {
                    int idx = Arrays.binarySearch(times, estimatedTime);
                    if (idx < -1) {
                        idx =  -idx - 2;
                    } else if (idx == -1) {
                        idx = 0;
                    }
                    depth = 2 * idx + 1;
                }
                if (round < 8) {
                    depth = Math.min(3, depth);
                } else if (round < 12) {
                    depth = Math.min(7, depth);
                }
            } else {
                depth = 1;
            }
            this.depth = depth;
            this.utilityWeight = GameCheckersAgent.UTILITY_WEIGHT;

            this.writeState(GameCheckersAgent.STATE_FILE, round + 1, GameCheckersAgent.DEFAULT_CHARSET);
        }

        public GameCheckersAgent(final CheckersBoard.Color myColor, final CheckersBoard board, final int depth,
                final int[] utilityWeight) {
            super(myColor, board);
            this.depth = depth;
            this.utilityWeight = utilityWeight != null ? utilityWeight : GameCheckersAgent.UTILITY_WEIGHT;
        }

        @Override
        public CheckersBoard.Action next() {
            if (this.depth <= 0) {
                return CheckersBoard.Action.EMPTY_ACTION;
            }

            final Queue<CheckersBoard.Action> actions = this.expand(
                    new PriorityQueue<>(CheckersBoard.MAX_NUM_MOVES), this.myColor);
            if (actions.isEmpty()) {
                return CheckersBoard.Action.EMPTY_ACTION;
            }

            int alpha = Integer.MIN_VALUE, beta = Integer.MAX_VALUE;
            CheckersBoard.Action result = CheckersBoard.Action.EMPTY_ACTION;
            int resultUtility = Integer.MIN_VALUE;
            for (final CheckersBoard.Action action : actions) {
                this.board.doAction(action);
                final int utility = this.min(this.myColor.next(), this.depth - 1, alpha, beta);
                this.board.undoAction(action);

                if (resultUtility >= utility) {
                    continue;
                }
                resultUtility = utility;
                result = action;
                alpha = Math.max(resultUtility, alpha);
            }
            return result;
        }

        private int max(final CheckersBoard.Color color, final int depth, int alpha, final int beta) {
            if (depth <= 0) {
                return this.evaluateUtility();
            }

            final Queue<CheckersBoard.Action> actions = this.expand(
                    new PriorityQueue<>(CheckersBoard.MAX_NUM_MOVES), color);
            if (actions.isEmpty()) {
                return this.evaluateUtility();
            }

            int resultUtility = Integer.MIN_VALUE;
            for (final CheckersBoard.Action action : actions) {
                this.board.doAction(action);
                final int utility = this.min(color.next(), depth - 1, alpha, beta);
                this.board.undoAction(action);

                if (resultUtility >= utility) {
                    continue;
                }
                resultUtility = utility;
                if (resultUtility >= beta) {
                    return resultUtility;
                }
                alpha = Math.max(resultUtility, alpha);
            }
            return resultUtility;
        }

        private int min(final CheckersBoard.Color color, final int depth, final int alpha, int beta) {
            if (depth <= 0) {
                return this.evaluateUtility();
            }

            final Queue<CheckersBoard.Action> actions = this.expand(
                    new PriorityQueue<>(CheckersBoard.MAX_NUM_MOVES), color);
            if (actions.isEmpty()) {
                return this.evaluateUtility();
            }

            int resultUtility = Integer.MAX_VALUE;
            for (final CheckersBoard.Action action : actions) {
                this.board.doAction(action);
                final int utility = this.max(color.next(), depth - 1, alpha, beta);
                this.board.undoAction(action);

                if (resultUtility <= utility) {
                    continue;
                }
                resultUtility = utility;
                if (resultUtility <= alpha) {
                    return resultUtility;
                }
                beta = Math.min(resultUtility, beta);
            }
            return resultUtility;
        }

        private int evaluateUtility() {
            if (this.myColor == CheckersBoard.Color.WHITE) {
                return -this.evaluateBlackUtility();
            }
            return this.evaluateBlackUtility();
        }

        private int evaluateBlackUtility() {
            final Set<CheckersBoard.Point> blackPawnPts =
                    this.board.getPositionsByPiece(CheckersBoard.Piece.BLACK_PAWN);
            final Set<CheckersBoard.Point> whitePawnPts =
                    this.board.getPositionsByPiece(CheckersBoard.Piece.WHITE_PAWN);
            final Set<CheckersBoard.Point> blackKingPts =
                    this.board.getPositionsByPiece(CheckersBoard.Piece.BLACK_KING);
            final Set<CheckersBoard.Point> whiteKingPts =
                    this.board.getPositionsByPiece(CheckersBoard.Piece.WHITE_KING);

            if (blackPawnPts.isEmpty() && blackKingPts.isEmpty()) {
                return Integer.MIN_VALUE + 1;
            } else if (whitePawnPts.isEmpty() && whiteKingPts.isEmpty()) {
                return Integer.MAX_VALUE - 1;
            }

            int numPawns = blackPawnPts.size() - whitePawnPts.size();
            int numKings = blackKingPts.size() - whiteKingPts.size();

            int numSafePawns = 0, numSafeKings = 0;
            for (final CheckersBoard.Point p : blackPawnPts) {
                if (p.isSafe) {
                    ++numSafePawns;
                }
            }
            for (final CheckersBoard.Point p : whitePawnPts) {
                if (p.isSafe) {
                    --numSafePawns;
                }
            }
            for (final CheckersBoard.Point p : blackKingPts) {
                if (p.isSafe) {
                    ++numSafeKings;
                }
            }
            for (final CheckersBoard.Point p : whiteKingPts) {
                if (p.isSafe) {
                    --numSafeKings;
                }
            }

            int numEmptyPromoLine = 0;
            for (int i = 0; i < CheckersBoard.SIZE; ++i) {
                if (this.board.pieces[CheckersBoard.Color.BLACK.touchdown][i] == CheckersBoard.Piece.EMPTY) {
                    ++numEmptyPromoLine;
                }
                if (this.board.pieces[CheckersBoard.Color.WHITE.touchdown][i] == CheckersBoard.Piece.EMPTY) {
                    --numEmptyPromoLine;
                }
            }

            int numDefenderPieces = 0;
            for (final CheckersBoard.Point p : blackPawnPts) {
                if (p.y < 2) {
                    ++numDefenderPieces;
                }
            }
            for (final CheckersBoard.Point p : blackKingPts) {
                if (p.y < 2) {
                    ++numDefenderPieces;
                }
            }
            for (final CheckersBoard.Point p : whitePawnPts) {
                if (p.y > CheckersBoard.SIZE - 3) {
                    --numDefenderPieces;
                }
            }
            for (final CheckersBoard.Point p : whiteKingPts) {
                if (p.y > CheckersBoard.SIZE - 3) {
                    --numDefenderPieces;
                }
            }

            int maxBlackRow = 0, minWhiteRow = CheckersBoard.SIZE;
            for (final CheckersBoard.Point p : blackPawnPts) {
                maxBlackRow = Math.max(p.y, maxBlackRow);
            }
            for (final CheckersBoard.Point p : whitePawnPts) {
                minWhiteRow = Math.min(p.y, minWhiteRow);

            }
            maxBlackRow -= 3;
            minWhiteRow += 3;
            int numAttackingPawns = 0;
            for (final CheckersBoard.Point p : blackPawnPts) {
                if (p.y > maxBlackRow) {
                    ++numAttackingPawns;
                }
            }
            for (final CheckersBoard.Point p : whitePawnPts) {
                if (p.y < minWhiteRow) {
                    --numAttackingPawns;
                }
            }

            int numCentralPawns = 0, numCentralKings = 0;
            for (final CheckersBoard.Point p : blackPawnPts) {
                if (p.isCentral) {
                    ++numCentralPawns;
                }
            }
            for (final CheckersBoard.Point p : whitePawnPts) {
                if (p.isCentral) {
                    --numCentralPawns;
                }
            }
            for (final CheckersBoard.Point p : blackKingPts) {
                if (p.isCentral) {
                    ++numCentralKings;
                }
            }
            for (final CheckersBoard.Point p : whiteKingPts) {
                if (p.isCentral) {
                    --numCentralKings;
                }
            }

            return this.utilityWeight[0] * numPawns +
                    this.utilityWeight[1] * numKings +
                    this.utilityWeight[2] * numSafePawns +
                    this.utilityWeight[3] * numSafeKings +
                    this.utilityWeight[4] * numEmptyPromoLine +
                    this.utilityWeight[5] * numDefenderPieces +
                    this.utilityWeight[6] * numAttackingPawns +
                    this.utilityWeight[7] * numCentralPawns +
                    this.utilityWeight[8] * numCentralKings;
        }

        private float[] readCalibration(final Path input, final Charset charset) {
            if (input == null || !Files.exists(input)) {
                return null;
            }
            try (Scanner scanner = new Scanner(Files.newBufferedReader(input, charset))) {
                final List<Float> timeList = new ArrayList<>();
                while (scanner.hasNextLine()) {
                    timeList.add(Float.valueOf(scanner.nextLine()));
                }
                final float[] times = new float[timeList.size()];
                int idx = 0;
                for (final float time : timeList) {
                    times[idx++] = time;
                }
                return times;
            } catch (IOException e) {
                return null;
            }
        }

        private int readState(final Path input, final Charset charset) {
            if (!Files.exists(input)) {
                return 0;
            }
            try (Scanner scanner = new Scanner(Files.newBufferedReader(input, charset))) {
                final int round = scanner.nextInt();
                return round;
            } catch (IOException e) {
                return 0;
            }
        }

        private void writeState(final Path output, final int round, final Charset charset) {
            try (Writer writer = Files.newBufferedWriter(output, charset)) {
                writer.append(Integer.toString(round));
            } catch (IOException e) {
                return;
            }
        }

    }

    public static class CheckersSetup {

        public static enum Type {
            SINGLE(setup -> new SingleMoveCheckersAgent(setup.myColor, setup.board)),
            GAME(setup -> new GameCheckersAgent(setup.myColor, setup.board, setup.remainingTime)),
            DEBUG(setup -> new GameCheckersAgent(setup.myColor, setup.board, setup.depth, setup.utilityWeight));

            private Function<CheckersSetup, CheckersAgent> agentFactory;

            Type(Function<CheckersSetup, CheckersAgent> agentFactory) {
                this.agentFactory = agentFactory;
            }
        }

        private final CheckersSetup.Type type;
        private final CheckersBoard.Color myColor;
        private final float remainingTime;
        private final int depth;
        private final int[] utilityWeight;
        private final CheckersBoard board;

        public CheckersSetup(final String type, final String myColor, final float remainingTime, final int depth,
                final int[] utilityWeight, CheckersBoard board)  {
            this.type = CheckersSetup.Type.valueOf(type);
            this.myColor = CheckersBoard.Color.valueOf(myColor);
            this.remainingTime = remainingTime;
            this.depth = depth;
            this.utilityWeight = utilityWeight;
            this.board = board;
        }

        public CheckersSetup(final String type, final String myColor, final float remainingTime, final int depth) {
            this(type, myColor, remainingTime, depth, null, new CheckersBoard());
        }

        public CheckersAgent createAgent() {
            return this.type.agentFactory.apply(this);
        }

    }

    public CheckersBoard.Action solve(final Path output, final Path input, final Charset charset) {
        if (input == null) {
            return CheckersBoard.Action.EMPTY_ACTION;
        }
        final CheckersSetup setup = this.read(input, charset);
        final CheckersAgent agent = setup.createAgent();
        final CheckersBoard.Action result = agent.next();
        this.write(output, result, charset);
        return result;
    }

    private CheckersSetup read(final Path input, final Charset charset) {
        try (Scanner scanner = new Scanner(Files.newBufferedReader(input, charset))) {
            final String type = scanner.nextLine();
            final String myColor = scanner.nextLine();
            final float remainingTime = 1000 * Float.valueOf(scanner.nextLine());
            int depth = Solution.DEFAULT_SEARCH_DEPTH;
            if (CheckersSetup.Type.DEBUG.toString().equals(type)) {
                depth = Integer.valueOf(scanner.nextLine());
            }

            final CheckersSetup setup = new CheckersSetup(type, myColor, remainingTime, depth);
            final CheckersBoard board = setup.board;
            for (int i = 0; i < CheckersBoard.SIZE; ++i) {
                final String line = scanner.nextLine();
                for (int j = 0; j < CheckersBoard.SIZE; ++j) {
                    final CheckersBoard.Piece piece = CheckersBoard.Piece.of(line.charAt(j));
                    if (piece != CheckersBoard.Piece.EMPTY) {
                        board.setPiece(piece, CheckersBoard.Point.getPoint(j, i));
                    }
                }
            }
            return setup;
        } catch (IOException e) {
            return null;
        }
    }

    private void write(final Path output, final CheckersBoard.Action result, final Charset charset) {
        if (output == null) {
            return;
        } else if (result == null) {
            try {
                Files.createFile(output);
            } catch (IOException e) {
                return;
            }
        }
        try (Writer writer = Files.newBufferedWriter(output, charset)) {
            result.appendTo(writer, '\n');
        } catch (IOException e) {
            return;
        }
    }

}
