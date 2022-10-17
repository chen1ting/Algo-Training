# reference: https://www.pythonpool.com/viterbi-algorithm-python/

import numpy as np


def viterbi(states:list, features:list, start_p:list, trans_p, emit_p):
    
    # Initialization
    n_states=len(states)
    n_features=len(features)
    # construct a matrix with n_states rows and n_features columns
    V=np.zeros((n_states,n_features)) # initialize with zeros
    
    
    #Iteration
    for i in range(1,n_features): # for each column, take V from previous columns and compute
        possible_lst=emit_p


states=["<s>", "VB","TO","NN","PPSS"]
features=["I", "want", "to", "race"]
start_p=[1.0, 0.0, 0.0, 0.0, 0.0]
trans_p=np.array([
#    <s>    VB      TO      NN     PPSS   
    [.0,   .019,  .0043,   .041,  .067],     # <s>
    [.0,  .0038,   .035,   .047,  .007],     # VB
    [.0,    .83,      0, .00047,     0],     # TO
    [.0,   .004,   .016,   .087, .0045],     # NN
    [.0,    .23, .00079,  .0012, .00014]     # PPSS
])
emit_p=np.array([
#      I       want    to    race   
    [  0,     .0093,    0, .00012],     # VB
    [  0,         0,  .99,      0],     # TO
    [  0,   .000054,    0, .00057],     # NN
    [.37,         0,    0,      0]      # PPSS
])