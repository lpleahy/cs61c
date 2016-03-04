import sys
import re

from pyspark import SparkContext

def flat_map(document):
    """
    Takes in document, which is a key, value pair, where document[0] is the
    document ID and document[1] is the contents of the document.
    HINT: You need to keep track of three things, word, document ID, and the
    index inside of the document, but you are working with key, value pairs.
    Is there a way to combine these three things and make a key, value pair?
    """
    """ Your code here. """
    lst = re.findall(r"\w+", document[1]) 
    new_lst = []
    for i in range(len(lst)):
       new_lst.append((document[0], i, lst[i])) 
    return new_lst

def map(arg):
    """ Your code here. """
    return ((arg[0], arg[1]), arg[2]) 

def reduce(arg1, arg2):
    """ Your code here. """
    return (arg1[1], arg1[0][0], arg1[0][1], arg2[0][0], arg2[0][1])

def index(file_name, output="spark-wc-out-index"):
    sc = SparkContext("local[8]", "Index")
    file = sc.sequenceFile(file_name)

    indices = file.flatMap(flat_map) \
                  .map(map) \
                  .reduceByKey(reduce)

    indices.coalesce(1).saveAsTextFile(output)

""" Do not worry about this """
if __name__ == "__main__":
    argv = sys.argv
    if len(argv) == 2:
        index(argv[1])
    else:
        index(argv[1], argv[2])
