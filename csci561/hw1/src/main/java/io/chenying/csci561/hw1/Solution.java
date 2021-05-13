package io.chenying.csci561.hw1;

import java.io.BufferedWriter;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.PriorityQueue;
import java.util.Queue;
import java.util.Scanner;
import java.util.Set;
import java.util.function.Function;

public enum Solution {

    INSTANCE;

    private final static String FAIL = "FAIL";

    public static abstract class SearchAlgorithm<T extends SearchAlgorithm.Point> {

        public final static int STRAIGHT_MOVE_COST = 10;
        public final static int DIAGONAL_MOVE_COST = 14;

        public final static int[][] MOVES = new int[][]{
            new int[]{1, 0, STRAIGHT_MOVE_COST}, new int[]{1, 1, DIAGONAL_MOVE_COST},
            new int[]{0, 1, STRAIGHT_MOVE_COST}, new int[]{-1, 1, DIAGONAL_MOVE_COST},
            new int[]{-1, 0, STRAIGHT_MOVE_COST}, new int[]{-1, -1, DIAGONAL_MOVE_COST},
            new int[]{0, -1, STRAIGHT_MOVE_COST}, new int[]{1, -1, DIAGONAL_MOVE_COST}
        };

        public static class Point {
            private final int x;
            private final int y;

            private Point parent;
            private long cost;
            private boolean visited;

            public Point(final int x, final int y) {
                this.x = x;
                this.y = y;
                this.cost = Long.MAX_VALUE;
            }

            public int getX() {
                return this.x;
            }

            public int getY() {
                return this.y;
            }

            public Point getParent() {
                return this.parent;
            }

            public void setParent(final Point parent) {
                this.parent = parent;
            }

            public long getCost() {
                return this.cost;
            }

            public void setCost(final long cost) {
                this.cost = cost;
            }

            public boolean isVisited() {
                return this.visited;
            }

            public void visit() {
                this.visited = true;
            }

            @Override
            public int hashCode() {
                return Objects.hash(this.x, this.y);
            }

            @Override
            public boolean equals(Object obj) {
                if (obj instanceof Point) {
                    return this.x == ((Point) obj).x && this.y == ((Point) obj).y;
                }
                return false;
            }

            @Override
            public String toString() {
                return new StringBuilder(10).append(this.x).append(',').append(this.y).toString();
            }
        }

        private final int width;
        private final int height;
        private final int[][] map;

        private final List<T> points;

        private final T source;
        private final Map<T, List<Integer>> targets;
        private final int numTargets;

        private final int maxRockHeight;

        public SearchAlgorithm(final int[][] map, final Point source, final Point[] targets, final int maxRockHeight) {
            this.width = map[0].length;
            this.height = map.length;
            this.map = map;
            this.points = new ArrayList<>(this.width * this.height);
            for (int i = 0; i < this.width * this.height; ++i) {
                this.points.add(null);
            }
            this.source = this.getPoint(source.getX(), source.getY());
            Map<T, List<Integer>> tmpTargetsMap = new HashMap<>();
            for (int i = 0; i < targets.length; ++i) {
                T target = this.getPoint(targets[i].getX(), targets[i].getY());
                tmpTargetsMap.putIfAbsent(target, new LinkedList<>());
                tmpTargetsMap.get(target).add(i);
            }
            this.targets = tmpTargetsMap;
            this.numTargets = targets.length;
            this.maxRockHeight = maxRockHeight;
        }

        public List<List<Point>> search() {
            final List<List<Point>> result = this.getEmptyResult();
            if (this.targets.isEmpty()) {
                return result;
            }

            final Queue<T> queue = this.makeQueue(this.source);
            while (!queue.isEmpty()) {
                T current = queue.poll();
                if (current.isVisited()) {
                    continue;
                } else if (this.targets.containsKey(current)) {
                    List<Point> path = this.getPath(current);
                    for (int idx : this.targets.get(current)) {
                        result.set(idx, path);
                    }
                    this.targets.remove(current);
                    if (this.targets.isEmpty()) {
                        break;
                    }
                    this.onArrival(current, queue);
                }
                current.visit();

                for (final int[] move : SearchAlgorithm.MOVES) {
                    final int nextX = current.getX() + move[0], nextY = current.getY()+ move[1];
                    if (nextX < 0 || nextX >= this.width || nextY < 0 || nextY >= this.height) {
                        continue;
                    }

                    final int heightCost = Math.abs(Math.min(this.map[nextY][nextX], 0) -
                            Math.min(this.map[current.getY()][current.getX()], 0));
                    if (heightCost > this.maxRockHeight) {
                        continue;
                    }

                    final T next = this.getPoint(nextX, nextY);
                    if (next.isVisited()) {
                        continue;
                    }
                    final boolean isToExpand = this.expand(next, current,
                            move[2], Math.max(this.map[nextY][nextX], 0), heightCost, queue);
                    if (isToExpand) {
                        next.setParent(current);
                        queue.offer(next);
                    }
                }
            }
            return result;
        }

        protected abstract Queue<T> makeQueue(final T source);

        protected abstract boolean expand(final T next, final T current,
                final int moveCost, final int muddinessCost, final int heightCost,
                final Queue<T> queue);

        protected abstract T supplyPoint(final int x, final int y);

        protected void onArrival(final T target, final Queue<T> queue) {}

        protected int[][] getMap() {
            return this.map;
        }

        protected Set<T> getTargets() {
            return Optional.ofNullable(this.targets).map(Map::keySet).orElse(null);
        }

        private List<List<Point>> getEmptyResult() {
            List<List<Point>> result = new ArrayList<>(this.numTargets);
            for (int i = 0; i < this.numTargets; ++i) {
                result.add(null);
            }
            return result;
        }

        private T getPoint(final int x, final int y) {
            final int idx = y * this.width + x;
            if (this.points.get(idx) == null) {
                this.points.set(idx, this.supplyPoint(x, y));
            }
            return this.points.get(idx);
        }

        private List<Point> getPath(final Point target) {
            LinkedList<Point> path = new LinkedList<>();
            Point point = target;
            while (point != null) {
                path.offerFirst(point);
                point = point.getParent();
            }
            return path;
        }

    }

    private static class BreadthFirstSearchAlgorithm extends SearchAlgorithm<SearchAlgorithm.Point> {

        public BreadthFirstSearchAlgorithm(final int[][] map, final Point source,
                final Point[] targets, final int maxRockHeight) {
            super(map, source, targets, maxRockHeight);
        }

        @Override
        protected Queue<Point> makeQueue(final Point source) {
            Queue<Point> queue = new LinkedList<>();
            queue.offer(source);
            return queue;
        }

        @Override
        protected boolean expand(final Point next, final Point current,
                final int moveCost, final int muddinessCost, final int heightCost,
                final Queue<Point> queue) {
            return next.getParent() == null;
        }

        @Override
        protected Point supplyPoint(final int x, final int y) {
            return new Point(x, y);
        }

    }

    private static class UniformCostSearchAlgorithm extends SearchAlgorithm<SearchAlgorithm.Point> {

        public UniformCostSearchAlgorithm(final int[][] map, final Point source,
                final Point[] targets, final int maxRockHeight) {
            super(map, source, targets, maxRockHeight);
        }

        @Override
        protected Queue<Point> makeQueue(final Point source) {
            Queue<Point> queue = new PriorityQueue<>((x, y) -> Long.compare(x.getCost(), y.getCost()));
            source.setCost(0L);
            queue.offer(source);
            return queue;
        }

        @Override
        protected boolean expand(final Point next, final Point current,
                final int moveCost, final int muddinessCost, final int heightCost,
                final Queue<Point> queue) {
            final long cost = current.getCost() + moveCost;
            if (cost >= next.getCost()) {
                return false;
            }
            queue.remove(next);
            next.setCost(cost);
            return true;
        }

        @Override
        protected Point supplyPoint(final int x, final int y) {
            return new Point(x, y);
        }

    }

    private static class AStarSearchAlgorithm extends SearchAlgorithm<AStarSearchAlgorithm.HeuristicPoint> {

        private static class HeuristicPoint extends SearchAlgorithm.Point {
            private final Map<Point, Long> heuristicCosts;
            private long minHeuristicCost;

            public HeuristicPoint(final int x, final int y) {
                super(x, y);
                this.heuristicCosts = new HashMap<>();
                this.minHeuristicCost = Long.MAX_VALUE;
            }

            public long getMinHeuristicCost() {
                return this.minHeuristicCost;
            }

            public boolean isHeuristicCostsEmpty() {
                return this.heuristicCosts.isEmpty();
            }

            public void addHeuristicCost(final Point target, final long heuristicCost) {
                this.heuristicCosts.put(target, heuristicCost);
                this.minHeuristicCost = Math.min(heuristicCost, this.minHeuristicCost);
            }

            public void removeHeuristicCost(final Point target) {
                this.heuristicCosts.remove(target);
                long min = Long.MAX_VALUE;
                for (final long heuristicCost : this.heuristicCosts.values()) {
                    min = Math.min(heuristicCost, min);
                }
                this.minHeuristicCost = min;
            }

        }

        public AStarSearchAlgorithm(final int[][] map, final Point source,
                final Point[] targets, final int maxRockHeight) {
            super(map, source, targets, maxRockHeight);
        }

        @Override
        protected Queue<HeuristicPoint> makeQueue(final HeuristicPoint source) {
            this.getTargets().forEach(this::calculateHeuristicCosts);
            Queue<HeuristicPoint> queue = new PriorityQueue<>((x, y) ->
                    Long.compare(this.getTotalCost(x), this.getTotalCost(y)));
            source.setCost(0L);
            queue.offer(source);
            return queue;
        }

        @Override
        protected boolean expand(final HeuristicPoint next, final HeuristicPoint current,
                final int moveCost, final int muddinessCost, final int heightCost,
                final Queue<HeuristicPoint> queue) {
            final long cost = current.getCost() + moveCost + muddinessCost + heightCost;
            if (cost >= next.getCost()) {
                return false;
            }
            queue.remove(next);
            next.setCost(cost);
            return true;
        }

        @Override
        protected HeuristicPoint supplyPoint(final int x, final int y) {
            HeuristicPoint point = new HeuristicPoint(x, y);
            this.calculateHeuristicCosts(point);
            return point;
        }

        @Override
        protected void onArrival(final HeuristicPoint target, final Queue<HeuristicPoint> queue) {
            List<HeuristicPoint> duplicatedList = new ArrayList<>(queue);
            queue.clear();
            for (HeuristicPoint point : duplicatedList) {
                point.removeHeuristicCost(target);
            }
            queue.addAll(duplicatedList);
        }

        private void calculateHeuristicCosts(final HeuristicPoint point) {
            if (this.getTargets() == null || !point.isHeuristicCostsEmpty()) {
                return;
            }
            final int height = Math.max(-this.getMap()[point.getY()][point.getX()], 0);
            for (final Point target : this.getTargets()) {
                if (point == target) {
                    point.addHeuristicCost(target, 0L);
                    continue;
                }
                final int x = Math.abs(target.getX() - point.getX());
                final int y = Math.abs(target.getY() - point.getY());
                final long move = SearchAlgorithm.STRAIGHT_MOVE_COST * Math.max(x, y) +
                    (SearchAlgorithm.DIAGONAL_MOVE_COST - SearchAlgorithm.STRAIGHT_MOVE_COST) * Math.min(x, y);
                final int targetHeight = Math.max(-this.getMap()[target.getY()][target.getX()], 0);
                point.addHeuristicCost(target, move + Math.abs(targetHeight - height));
            }
        }

        private long getTotalCost(final HeuristicPoint point) {
            if (point.getMinHeuristicCost() >= Long.MAX_VALUE - point.getCost()) {
                return Long.MAX_VALUE;
            }
            return point.getCost() + point.getMinHeuristicCost();
        }

    }

    private static class SearchAlgorithmFactory {

        public static Map<String, Function<SearchAlgorithmFactory, SearchAlgorithm<?>>> searchAlgorithms;

        static {
            SearchAlgorithmFactory.searchAlgorithms = new HashMap<>();
            SearchAlgorithmFactory.searchAlgorithms.put("BFS",
                    f -> new BreadthFirstSearchAlgorithm(f.map, f.source, f.targets, f.maxRockHeight));
            SearchAlgorithmFactory.searchAlgorithms.put("UCS",
                    f -> new UniformCostSearchAlgorithm(f.map, f.source, f.targets, f.maxRockHeight));
            SearchAlgorithmFactory.searchAlgorithms.put("A*",
                    f -> new AStarSearchAlgorithm(f.map, f.source, f.targets, f.maxRockHeight));
        }

        String algorithm;
        SearchAlgorithm.Point source;
        int maxRockHeight;
        SearchAlgorithm.Point[] targets;
        int[][] map;

        public SearchAlgorithm<?> get() {
            if (!SearchAlgorithmFactory.searchAlgorithms.containsKey(this.algorithm)) {
                return null;
            }
            SearchAlgorithm<?> algorithm = SearchAlgorithmFactory.searchAlgorithms.get(this.algorithm).apply(this);
            return algorithm;
        }
    }

    public void solve(final Path output, final Path input, final Charset charset) {
        SearchAlgorithmFactory factory = Solution.INSTANCE.read(input, charset);
        if (factory == null) {
            Solution.INSTANCE.write(output, null, charset);
            return;
        }

        SearchAlgorithm<?> algorithm = factory.get();
        Solution.INSTANCE.write(output, algorithm.search(), charset);
    }

    private SearchAlgorithmFactory read(final Path input, final Charset charset) {
        final SearchAlgorithmFactory factory = new SearchAlgorithmFactory();
        try (Scanner scanner = new Scanner(Files.newBufferedReader(input, charset))) {
            // 1st line
            factory.algorithm = scanner.nextLine();
            if (!SearchAlgorithmFactory.searchAlgorithms.containsKey(factory.algorithm)) {
                return null;
            }

            // 2nd line
            final int width = scanner.nextInt();
            final int height = scanner.nextInt();
            if (width <= 0 || height <= 0) {
                return null;
            }

            // 3rd line
            int x = scanner.nextInt();
            int y = scanner.nextInt();
            if (x < 0 || x >= width || y < 0 || y >= height) {
                return null;
            }
            factory.source = new SearchAlgorithm.Point(x, y);

            // 4th line
            factory.maxRockHeight = scanner.nextInt();
            if (factory.maxRockHeight < 0) {
                return null;
            }

            // 5th line
            final int numTargets = scanner.nextInt();
            if (numTargets <= 0) {
                return null;
            }

            // next numTargets lines
            factory.targets = new SearchAlgorithm.Point[numTargets];
            for (int i = 0; i < numTargets; ++i) {
                x = scanner.nextInt();
                y = scanner.nextInt();
                if (x < 0 || x >= width || y < 0 || y >= height) {
                    return null;
                }
                factory.targets[i] = new SearchAlgorithm.Point(x, y);
            }

            // next height lines
            factory.map = new int[height][width];
            for (int i = 0; i < height; ++i) {
                for (int j = 0; j < width; ++j) {
                    factory.map[i][j] = scanner.nextInt();
                }
            }
            return factory;
        } catch (Exception e) {
            return null;
        }
    }

    private void write(final Path output, final List<List<SearchAlgorithm.Point>> result, final Charset charset) {
        if (output == null) {
            return;
        } else if (result == null || result.isEmpty()) {
            try {
                Files.createFile(output);
            } catch (Exception e) {
                return;
            }
            return;
        }
        try (BufferedWriter writer = Files.newBufferedWriter(output, charset)) {
            Iterator<List<SearchAlgorithm.Point>> resultIt = result.iterator();
            while (resultIt.hasNext()) {
                List<SearchAlgorithm.Point> path = resultIt.next();
                if (path == null || path.isEmpty()) {
                    writer.write(Solution.FAIL);
                } else {
                    Iterator<SearchAlgorithm.Point> it = path.iterator();
                    while (it.hasNext()) {
                        SearchAlgorithm.Point point = it.next();
                        writer.write(point.toString());
                        if (it.hasNext()) {
                            writer.write(' ');
                        }
                    }
                }
                if (resultIt.hasNext()) {
                    writer.append(System.lineSeparator());
                }
            }
        } catch (Exception e) {
            return;
        }
    }

}
