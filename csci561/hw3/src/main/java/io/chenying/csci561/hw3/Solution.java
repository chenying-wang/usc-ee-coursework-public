package io.chenying.csci561.hw3;

import java.io.Writer;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.PriorityQueue;
import java.util.Queue;
import java.util.Scanner;
import java.util.Set;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.regex.Pattern;

public enum Solution {

    INSTANCE;

    private final static Map<Boolean, String> OUTPUT_FORMAT = Map.of(
        false, "FALSE",
        true, "TRUE"
    );

    private static class ConstantPool<T> {
        private final List<T> list;
        private final Map<T, Integer> lookupTable;

        public ConstantPool() {
            this.list = new ArrayList<>();
            this.lookupTable = new HashMap<>();
        }

        public int add(final T constant) {
            if (this.lookupTable.containsKey(constant)) {
                return this.lookupTable.get(constant);
            }
            final int idx = list.size();
            this.list.add(constant);
            this.lookupTable.put(constant, idx);
            return idx;
        }

        public void clear() {
            this.list.clear();
            this.lookupTable.clear();
        }
    }

    private static class DisjointSets {
        private final Map<Integer, Integer> parent;

        public DisjointSets() {
            this.parent = new HashMap<>();
        }

        public int find(final int x) {
            if (this.parent.getOrDefault(x, x) == x) {
                return x;
            }
            final int result = this.find(this.parent.get(x));
            this.parent.put(x, result);
            return result;
        }

        public void union(final int p, final int q) {
            if (p == q) {
                return;
            }
            final int pParent = this.find(p);
            final int qParent = this.find(q);
            if (pParent == qParent) {
                return;
            } else if (pParent < qParent) {
                this.parent.put(qParent, pParent);
            } else {
                this.parent.put(pParent, qParent);
            }
        }
    }

    private static class Utils {

        public final static String IMPLICATION = "=>";
        public final static String CONJUNCTION = "&";
        public final static String TERMS_SEPARATOR = ",";

        private final static String WHITESPACE_PATTERN = "\\s*";

        private static Pattern implicationSentenceSplitter = Pattern.compile(
                new StringBuilder(Utils.WHITESPACE_PATTERN)
                    .append(Utils.IMPLICATION)
                    .append(Utils.WHITESPACE_PATTERN).toString());

        private static Pattern conjunctionSplitter = Pattern.compile(
                new StringBuilder(Utils.WHITESPACE_PATTERN)
                    .append(Utils.CONJUNCTION)
                    .append(Utils.WHITESPACE_PATTERN).toString());

        private static Pattern termsSplitter = Pattern.compile(
                new StringBuilder(Utils.WHITESPACE_PATTERN)
                    .append(Utils.TERMS_SEPARATOR)
                    .append(Utils.WHITESPACE_PATTERN).toString());

        public static String[] splitImplicationSentence(final String implicationSentence) {
            if (!implicationSentence.contains(Utils.IMPLICATION)) {
                return new String[]{implicationSentence};
            }
            return Utils.implicationSentenceSplitter.split(implicationSentence, 0);
        }

        public static String[] splitConjunction(final String conjunction) {
            if (!conjunction.contains(Utils.CONJUNCTION)) {
                return new String[]{conjunction};
            }
            return Utils.conjunctionSplitter.split(conjunction, 0);
        }

        public static String[] splitAtomicSentence(final String atomicSentence) {
            final int leftIndex = atomicSentence.indexOf('(', 1);
            final int rightIndex = atomicSentence.indexOf(')', 3);
            if (leftIndex < 0 || rightIndex < 0) {
                return null;
            }
            final String[] fields = new String[2];
            fields[0] = atomicSentence.substring(0, leftIndex);
            fields[1] = atomicSentence.substring(leftIndex + 1, rightIndex);
            return fields;
        }

        public static String[] splitTerms(final String terms) {
            if (!terms.contains(Utils.TERMS_SEPARATOR)) {
                return new String[]{terms};
            }
            return Utils.termsSplitter.split(terms, 0);
        }

        public static boolean isConstant(final String term) {
            return Character.isUpperCase(term.charAt(0));
        }

    }

    private static class Clause implements Comparable<Clause> {

        public static class Literal {
            private boolean positive;
            private final int predicate;
            private final int[] terms;

            public Literal(final boolean positive, final int predicate, final int[] terms) {
                this.positive = positive;
                this.predicate = predicate;
                this.terms = terms;
            }

            public Literal(final Literal literal) {
                this.positive = literal.positive;
                this.predicate = literal.predicate;
                this.terms = Arrays.copyOf(literal.terms, literal.terms.length);
            }
        }

        private final Map<Integer, List<Clause.Literal>> predicates;
        private int size;

        public Clause() {
            this.predicates = new HashMap<>();
        }

        public static Clause parse(final String sentence, final boolean reverse,
                final Solution.ConstantPool<String> predicatePool,
                final Solution.ConstantPool<String> constantPool) {
            final Solution.ConstantPool<String> variablePool =  new Solution.ConstantPool<>();
            final String trimmedSentence = sentence.trim();
            if (!trimmedSentence.contains(Utils.IMPLICATION)) {
                final String[] literals = Utils.splitConjunction(trimmedSentence);
                final Solution.Clause clause = new Solution.Clause();
                for (final String literal : literals) {
                    clause.append(literal, reverse, predicatePool, constantPool, variablePool);
                }
                return clause;
            }
            final String[] fields = Utils.splitImplicationSentence(trimmedSentence);
            final String[] literals = Utils.splitConjunction(fields[0]);
            final Solution.Clause clause = new Solution.Clause();
            for (final String literal : literals) {
                clause.append(literal, !reverse, predicatePool, constantPool, variablePool);
            }
            return clause.append(fields[1], reverse, predicatePool, constantPool, variablePool);
        }

        public static Clause tryResolve(final Clause p, final Clause q, final int predicate,
                final int pIdx, final int qIdx) {
            final Solution.Clause.Literal pl = p.get(predicate).get(pIdx);
            final Solution.Clause.Literal ql = q.get(predicate).get(qIdx);
            if (pl.positive == ql.positive) {
                return null;
            }

            int offset = -1;
            for (final int pred : p.getPredicates()) {
                for (final Clause.Literal l : p.get(pred)) {
                    for (final int t : l.terms) {
                        offset = Math.max(t, offset);
                    }
                }
            }
            ++offset;

            final DisjointSets disjointSets = new DisjointSets();
            for (int i = 0; i < pl.terms.length; ++i) {
                int pTerm = pl.terms[i];
                int qTerm = ql.terms[i];
                if (pTerm < 0 && qTerm < 0) {
                    if (pTerm != qTerm) {
                        return null;
                    }
                    continue;
                } else if (qTerm >= 0) {
                    qTerm += offset;
                }

                int pGroup = disjointSets.find(pTerm);
                int qGroup = disjointSets.find(qTerm);
                if (pGroup < 0 && qGroup < 0 && pGroup != qGroup) {
                    return null;
                }
                disjointSets.union(pGroup, qGroup);
            }

            final Solution.ConstantPool<Integer> variablePool = new Solution.ConstantPool<>();
            final Solution.Clause resolvent = new Solution.Clause();
            for (final int pred : p.getPredicates()) {
                for (int i = 0; i < p.get(pred).size(); ++i) {
                    if (pred == predicate && i == pIdx) {
                        continue;
                    }
                    Clause.Literal literal = new Clause.Literal(p.get(pred).get(i));
                    for (int j = 0; j < literal.terms.length; ++j) {
                        if (literal.terms[j] < 0) {
                            continue;
                        }
                        int term = disjointSets.find(literal.terms[j]);
                        if (term < 0) {
                            literal.terms[j] = term;
                        } else {
                            literal.terms[j] = variablePool.add(term);
                        }
                    }
                    resolvent.addLiteral(literal);
                }
            }
            for (final int pred : q.getPredicates()) {
                for (int i = 0; i < q.get(pred).size(); ++i) {
                    if (pred == predicate && i == qIdx) {
                        continue;
                    }
                    Clause.Literal literal = new Clause.Literal(q.get(pred).get(i));
                    for (int j = 0; j < literal.terms.length; ++j) {
                        if (literal.terms[j] < 0) {
                            continue;
                        }
                        int term = disjointSets.find(literal.terms[j] + offset);
                        if (term < 0) {
                            literal.terms[j] = term;
                        } else {
                            literal.terms[j] = variablePool.add(term);
                        }
                    }
                    resolvent.addLiteral(literal);
                }
            }

            if (p.equals(resolvent) || q.equals(resolvent)) {
                return null;
            }
            return resolvent;
        }

        public Set<Integer> getPredicates() {
            return this.predicates.keySet();
        }

        public List<Clause.Literal> get(final int predicate) {
            return this.predicates.getOrDefault(predicate, List.of());
        }

        public boolean contains(final int predicate) {
            return this.predicates.containsKey(predicate);
        }

        public boolean isEmpty() {
            return this.predicates.isEmpty();
        }

        @Override
        public int compareTo(Clause o) {
            if (this.size < o.size) {
                return -1;
            } else if (this.size > o.size) {
                return 1;
            }
            return Integer.compare(this.predicates.size(), o.predicates.size());
        }

        @Override
        public boolean equals(Object obj) {
            if (!(obj instanceof Clause)) {
                return false;
            }

            final Clause clause = (Clause) obj;
            if (this == obj) {
                return true;
            } else if (this.size != clause.size || this.predicates.size() != clause.predicates.size()) {
                return false;
            }

            for (final int pred : this.getPredicates()) {
                int size = this.get(pred).size();
                if (!clause.contains(pred) || size != clause.get(pred).size()) {
                    return false;
                }
                int count = 0;
                for (int i = 0; i < size; ++i) {
                    if (this.get(pred).get(i).positive) {
                        ++count;
                    }
                    if (clause.get(pred).get(i).positive) {
                        --count;
                    }
                }
                if (count != 0) {
                    return false;
                }
            }
            return this.equalsSearch(clause);
        }

        private Clause append(final String literal, final boolean reverse,
                final Solution.ConstantPool<String> predicatePool, final Solution.ConstantPool<String> constantPool,
                final Solution.ConstantPool<String> variablePool) {
            final String[] fields = Utils.splitAtomicSentence(literal);
            final String[] strTerms = Utils.splitTerms(fields[1]);
            final int[] terms = new int[strTerms.length];
            for (int i = 0; i < strTerms.length; ++i) {
                if (Utils.isConstant(strTerms[i])) {
                    terms[i] = -constantPool.add(strTerms[i]) - 1;
                } else {
                    terms[i] = variablePool.add(strTerms[i]);
                }
            }
            Clause.Literal l = null;
            if (fields[0].charAt(0) == '~') {
                l = new Clause.Literal(reverse, predicatePool.add(fields[0].substring(1)), terms);
            } else {
                l = new Clause.Literal(!reverse, predicatePool.add(fields[0]), terms);
            }
            this.addLiteral(l);
            return this;
        }

        private void addLiteral(final Clause.Literal literal) {
            if (!this.predicates.containsKey(literal.predicate)) {
                this.predicates.put(literal.predicate, new ArrayList<>(4));
            }
            this.predicates.get(literal.predicate).add(literal);
            ++this.size;
        }

        private boolean equalsSearch(final Clause c) {
            final Integer[] arr = this.getPredicates().toArray(new Integer[0]);
            final int[] predicates = new int[arr.length];
            for (int i = 0 ; i < arr.length; ++i) {
                predicates[i] = arr[i];
            }
            return this.equalsSearch(c, predicates, 0, 0, new HashMap<>(), new HashMap<>(), null);
        }

        private boolean equalsSearch(final Clause c, final int[] predicates, final int predIdx, final int literalIdx,
                final Map<Integer, Integer> from, final Map<Integer, Integer> to, final boolean[] visited) {
            if (predIdx >= predicates.length) {
                return true;
            }

            final int predicate = predicates[predIdx];
            final int size = this.get(predicate).size();
            boolean[] v = visited;
            if (literalIdx >= size) {
                return this.equalsSearch(c, predicates, predIdx + 1, 0, from, to, null);
            } else if (literalIdx == 0) {
                v = new boolean[size];
            }

            final Clause.Literal literal = this.get(predicate).get(literalIdx);
            literalsLoop: for (int i = 0; i < size; ++i) {
                if (v[i]) {
                    continue;
                }

                Clause.Literal cLiteral = c.get(predicate).get(i);
                if (literal.positive != cLiteral.positive) {
                    continue;
                }

                Map<Integer, Integer> copyFrom = new HashMap<>(from);
                Map<Integer, Integer> copyTo = new HashMap<>(to);
                for (int j = 0; j < literal.terms.length; ++j) {
                    int thisTerm = literal.terms[j];
                    int cTerm = cLiteral.terms[j];
                    if (thisTerm < 0) {
                        if (thisTerm != cTerm) {
                            continue literalsLoop;
                        }
                    } else {
                        if (cTerm < 0 ||
                                copyFrom.getOrDefault(thisTerm, cTerm) != cTerm ||
                                copyTo.getOrDefault(cTerm, thisTerm) != thisTerm) {
                            continue literalsLoop;
                        }
                        copyFrom.put(thisTerm, cTerm);
                        copyTo.put(cTerm, thisTerm);
                    }
                }

                v[i] = true;
                boolean result = this.equalsSearch(c, predicates, predIdx, literalIdx + 1, copyFrom, copyTo, v);
                if (result) {
                    return true;
                }
                v[i] = false;
            }
            return false;
        }

    }

    private static class FOLKnowledgeBase {
        private final List<Solution.Clause> clauses;
        private final Solution.ConstantPool<String> predicatePool;
        private final Solution.ConstantPool<String> constantPool;
        private int maxClauseSize;

        public FOLKnowledgeBase() {
            this.clauses = new ArrayList<>();
            this.predicatePool = new Solution.ConstantPool<>();
            this.constantPool = new Solution.ConstantPool<>();
            this.maxClauseSize = 1;
        }

        public void appendSentence(final String sentence) {
            final Solution.Clause clause = Solution.Clause.parse(
                    sentence, false, this.predicatePool, this.constantPool);
            this.clauses.add(clause);
            this.maxClauseSize = Math.max(clause.size, this.maxClauseSize);
        }

        public void clear() {
            this.clauses.clear();
            this.predicatePool.clear();
            this.constantPool.clear();
            this.maxClauseSize = 1;
        }
    }

    private static class FOLResolution {

        private final static int MAX_NUM_INIT_CLAUSES = 50000;
        private final static int MAX_NUM_CLAUSES = 80000;

        private final Solution.FOLKnowledgeBase knowledgeBase;
        private final int clauseSizeThreshold;
        private final int initIndex;
        private final ExecutorService executor;

        public FOLResolution(final Solution.FOLKnowledgeBase knowledgeBase) {
            this.knowledgeBase = knowledgeBase;
            this.clauseSizeThreshold = knowledgeBase.maxClauseSize - 1;
            this.initIndex = this.init(10);
            this.executor = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());
        }

        public List<Boolean> query(final List<Solution.Clause> negQueries) {
            final int size = negQueries.size();
            final List<Future<Boolean>> futures = new ArrayList<>(size);
            for (final Solution.Clause negQueryClause : negQueries) {
                futures.add(new CompletableFuture<Boolean>().completeAsync(() -> {
                    return this.query(negQueryClause);
                }, this.executor));
            }
            final List<Boolean> results = new ArrayList<>(size);
            try {
                for (int i = 0; i < size; ++i) {
                    results.add(futures.get(i).get());
                }
            } catch (Exception e) {}
            return results;
        }

        public void shutdown() {
            this.executor.shutdown();
            this.knowledgeBase.clear();
        }

        private int init(final int round) {
            int start = 0, end = this.knowledgeBase.clauses.size();
            for (int r = 0; r < round; ++r) {
                end = this.knowledgeBase.clauses.size();
                if (end == start) {
                    break;
                }
                for (int i = start; i < end; ++i) {
                    for (int j = 0; j <= i; ++j) {
                        final Solution.Clause c1 = this.knowledgeBase.clauses.get(j);
                        final Solution.Clause c2 = this.knowledgeBase.clauses.get(i);
                        final List<Solution.Clause> resolvents = this.resolve(c1, c2);
                        if (resolvents.isEmpty()) {
                            continue;
                        }
                        resolventsLoop: for (final Solution.Clause resolvent : resolvents) {
                            for (final Solution.Clause c : this.knowledgeBase.clauses) {
                                if (c.equals(resolvent)) {
                                    continue resolventsLoop;
                                }
                            }
                            this.knowledgeBase.clauses.add(resolvent);
                        }
                    }
                    if (this.knowledgeBase.clauses.size() >= FOLResolution.MAX_NUM_INIT_CLAUSES) {
                        return i + 1;
                    }
                }
                start = end;
            }
            return end;
        }

        private boolean query(final Solution.Clause negQueryClause) {
            final List<Solution.Clause> existingClauses = new ArrayList<>();
            final Queue<Solution.Clause> queue = new PriorityQueue<>((x, y) -> x.compareTo(y));
            for (int i = 0; i < this.initIndex; ++i) {
                existingClauses.add(this.knowledgeBase.clauses.get(i));
            }
            for (int i = this.initIndex; i < this.knowledgeBase.clauses.size(); ++i) {
                queue.offer(this.knowledgeBase.clauses.get(i));
            }
            queue.offer(negQueryClause);

            while (!queue.isEmpty() && existingClauses.size() < FOLResolution.MAX_NUM_CLAUSES) {
                final Solution.Clause currentClause = queue.poll();
                for (final Solution.Clause existingClause : existingClauses) {
                    final List<Solution.Clause> resolvents = this.resolve(currentClause, existingClause);
                    if (resolvents.isEmpty()) {
                        continue;
                    }
                    for (final Solution.Clause resolvent : resolvents) {
                        if (resolvent.isEmpty()) {
                            return true;
                        }
                    }
                    resolventsLoop: for (final Solution.Clause resolvent : resolvents) {
                        for (final Solution.Clause c : existingClauses) {
                            if (c.equals(resolvent)) {
                                continue resolventsLoop;
                            }
                        }
                        for (final Solution.Clause c : queue) {
                            if (c.equals(resolvent)) {
                                continue resolventsLoop;
                            }
                        }
                        queue.offer(resolvent);
                    }
                }
                existingClauses.add(currentClause);
            }
            return false;
        }

        private List<Solution.Clause> resolve(final Solution.Clause p, final Solution.Clause q) {
            if (p.size + q.size - 2 > this.clauseSizeThreshold) {
                return List.of();
            }
            final List<Solution.Clause> resolvents = new ArrayList<>();
            for (final int predicate : p.getPredicates()) {
                if (!q.contains(predicate)) {
                    continue;
                }
                for (int i = 0; i < p.get(predicate).size(); ++i) {
                    qLiteralsLoop: for (int j = 0; j < q.get(predicate).size(); ++j) {
                        Clause resolvent = Clause.tryResolve(p, q, predicate, i, j);
                        if (resolvent == null) {
                            continue;
                        }
                        for (final Clause c : resolvents) {
                            if (c.equals(resolvent)) {
                                continue qLiteralsLoop;
                            }
                        }
                        resolvents.add(resolvent);
                    }
                }
            }
            return resolvents;
        }

    }

    private static class InputParameter {
        private final List<Solution.Clause> negQueries;
        private final Solution.FOLKnowledgeBase knowledgeBase;

        public InputParameter(final List<Solution.Clause> negQueries, final Solution.FOLKnowledgeBase knowledgeBase) {
            this.negQueries = negQueries;
            this.knowledgeBase = knowledgeBase;
        }
    }

    public void solve(final Path output, final Path input, final Charset charset) {
        if (input == null) {
            return;
        }
        final Solution.InputParameter inputParameter = this.read(input, charset);
        final Solution.FOLResolution resolution = new Solution.FOLResolution(inputParameter.knowledgeBase);
        final List<Boolean> results = resolution.query(inputParameter.negQueries);
        resolution.shutdown();
        this.write(output, results, charset);
    }

    private Solution.InputParameter read(final Path input, final Charset charset) {
        try (Scanner scanner = new Scanner(Files.newBufferedReader(input, charset))) {
            final int numQueries = Integer.valueOf(scanner.nextLine());
            final List<Solution.Clause> negQueries = new ArrayList<>(numQueries);
            final Solution.FOLKnowledgeBase knowledgeBase = new Solution.FOLKnowledgeBase();
            String line = null;
            for (int i = 0; i < numQueries; ++i) {
                line = scanner.nextLine();
                negQueries.add(Solution.Clause.parse(line, true, knowledgeBase.predicatePool, knowledgeBase.constantPool));
            }

            final int numSentences = Integer.valueOf(scanner.nextLine());
            for (int i = 0; i < numSentences; ++i) {
                line = scanner.nextLine();
                knowledgeBase.appendSentence(line);
            }
            return new Solution.InputParameter(negQueries, knowledgeBase);
        } catch (Exception e) {
            return null;
        }
    }

    private void write(final Path output, final List<Boolean> results, final Charset charset) {
        if (output == null) {
            return;
        } else if (results == null || results.isEmpty()) {
            try {
                Files.createFile(output);
            } catch (Exception e) {
                return;
            }
            return;
        }
        try (Writer writer = Files.newBufferedWriter(output, charset)) {
            writer.append(Solution.OUTPUT_FORMAT.get(results.get(0)));
            for (int i = 1; i < results.size(); ++i) {
                writer.append('\n').append(Solution.OUTPUT_FORMAT.get(results.get(i)));
            }
        } catch (Exception e) {
            return;
        }
    }

}
