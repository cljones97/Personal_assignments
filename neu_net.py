from numpy import loadtxt
from keras.models import Sequential
from keras.layers import Dense
from keras.wrappers.scikit_learn import KerasClassifier
from keras.utils import np_utils
from sklearn.model_selection import cross_val_score
from sklearn.model_selection import KFold
from sklearn.preprocessing import LabelEncoder
from sklearn.pipeline import Pipeline
from sklearn.preprocessing import MinMaxScaler
# load the dataset
dataset = loadtxt('wine_train.csv', delimiter=',')
# split into input (X) and output (y) variables
Y = dataset[:,0]
X = dataset[:,1:14]
# encode class values as integers
encoder = LabelEncoder()
encoder.fit(Y)
encoded_Y = encoder.transform(Y)
# convert integers to dummy variables (i.e. one hot encoded)
dummy_y = np_utils.to_categorical(encoded_Y)

scaler = MinMaxScaler()
idk = scaler.fit(X)
dataset2 = scaler.transform(X)

def get_model():
    # define the keras model
    model = Sequential()
    model.add(Dense(15, input_dim=13, activation='sigmoid'))
    model.add(Dense(8, activation='sigmoid'))
    #model.add(Dense(8, activation='softsign'))
    #model.add(Dense(5, activation='softplus'))
    model.add(Dense(3, activation='softmax'))
    # compile the keras model
    
    model.compile(loss='categorical_crossentropy', optimizer='adam', metrics=['accuracy'])
    model.fit(dataset2, dummy_y, epochs=300, batch_size=10, verbose=0)
    
    return model
#estimator = KerasClassifier(build_fn=baseline_model, epochs=600, batch_size=5, verbose=0)
model = get_model()
_, accuracy = model.evaluate(dataset2, dummy_y, verbose=0)
print('Accuracy: %.2f' % (accuracy*100))
"""
kfold = KFold(n_splits=10, shuffle=True)
results = cross_val_score(model, X, dummy_y, cv=kfold)
print("Baseline: %.2f%% (%.2f%%)" % (results.mean()*100, results.std()*100))
# fit the keras model on the dataset
model.fit(X, Y, epochs=150, batch_size=10, verbose=0)
"""
# evaluate the keras model
