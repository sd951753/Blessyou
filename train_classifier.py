import numpy as np
from os.path import basename
from glob import glob
from sklearn.svm import SVC
from sklearn.svm import LinearSVC
from sklearn import tree
from sklearn.ensemble import RandomForestClassifier
from sklearn.naive_bayes import GaussianNB
from micromlgen import port
from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import train_test_split

def load_features(folder):
    dataset = None
    classmap = {}
    for class_idx, filename in enumerate(glob('%s/*.csv' % folder)):
        class_name = basename(filename)[:-4]
        classmap[class_idx] = class_name
        samples = np.loadtxt(filename, dtype=float, delimiter=',')
        labels = np.ones((len(samples), 1)) * class_idx
        '''if(filename == 'sneeze.csv'): 
        	labels = np.ones((len(samples), 1)) * 1
        else :
        	labels = np.ones((len(samples), 1)) * 2'''
        samples = np.hstack((samples, labels))
        dataset = samples if dataset is None else np.vstack((dataset, samples))
    return dataset, classmap

np.random.seed(0)
dataset, classmap = load_features('data')
X, y = dataset[:, :-1], dataset[:, -1]
# this line is for testing your accuracy only: once you're satisfied with the results, set test_size to 1
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2)

#clf = SVC(kernel='rbf', gamma = 0.2, C=100) #0.5
#clf = tree.DecisionTreeClassifier() #0.89
clf = RandomForestClassifier(max_depth=3, random_state=0) #0.96
#clf = GaussianNB() #0.92
#clf = LogisticRegression(random_state=0) #0.85

clf.fit(X_train, y_train)

#print('Accuracy', clf.score(X_test, y_test))
print('Exported classifier to plain C')
print(port(clf, classmap=classmap))
print('Accuracy', clf.score(X_test, y_test))