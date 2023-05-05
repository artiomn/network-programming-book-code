import re

from mrjob.job import MRJob

WORD_RE = re.compile(r"[\w']+")


class MRWordFrequencyCount(MRJob):
    def mapper(self, _, line):
        for word in WORD_RE.findall(line):
            yield word.lower(), 1

    def combiner(self, word, counts):
        yield word, sum(counts)

    def reducer(self, word, counts):
        yield word, sum(counts)


if '__main__' == __name__:
    MRWordFrequencyCount.run()
