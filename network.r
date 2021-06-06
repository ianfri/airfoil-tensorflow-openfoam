# need to open the Anaconda prompt and type "conda activate tf" before opening RStudio

library(reticulate)
library(keras)

foilData = read.csv("foilData.csv",header=FALSE)
foilData = foilData[1:1000,1:3] # use 1st 1000 lines and 1st 3 columns
clData = read.csv("clData.csv",header=FALSE)

train_data = foilData[1:700,]
train_labels = clData[1:700,]
test_data = foilData[701:1000,]
test_labels = clData[701:1000,]

# Test data is *not* used when calculating the mean and std.

# Normalize training data
train_data <- scale(train_data) 

# Use means and standard deviations from training set to normalize test set
col_means_train <- attr(train_data, "scaled:center") 
col_stddevs_train <- attr(train_data, "scaled:scale")
test_data <- scale(test_data, center = col_means_train, scale = col_stddevs_train)

# build the model
build_model <- function() {
    
    model <- keras_model_sequential() %>%
        layer_dense(units = 12, activation = "sigmoid",
                    input_shape = dim(train_data)[2]) %>%
        layer_dense(units = 24, activation = "sigmoid") %>%
        layer_dense(units = 1)
    
    model %>% compile(
        loss = "mse",
        optimizer = optimizer_rmsprop(),
        metrics = list("mean_absolute_error")
    )
    
    model
}

model <- build_model()
model %>% summary()

# Display training progress by printing a single dot for each completed epoch.
print_dot_callback <- callback_lambda(
    on_epoch_end = function(epoch, logs) {
        if (epoch %% 80 == 0) cat("\n")
        cat(".")
    }
)    

epochs <- 400

# Fit the model and store training stats
history <- model %>% fit(
    train_data,
    train_labels,
    epochs = epochs,
    validation_split = 0.2,
    verbose = 0,
    callbacks = list(print_dot_callback)
)

library(ggplot2)

plot(history, metrics = "mean_absolute_error", smooth = FALSE) +
    coord_cartesian(ylim = c(0, 0.1))

# see how well the model performs on the test set
c(loss, mae) %<-% (model %>% evaluate(test_data, test_labels, verbose = 0))
paste0("Mean absolute error on test set: ", sprintf("%.6f", mae))

# predict some lift coefficients using data in the testing set
test_predictions = model %>% predict(test_data)
test_predictions[,1]


# -----------------------------------------------------------------------------------------


# plot(foilData[,1], clData[,1], xlab="Camber as %Chord", ylab="Lift Coefficient")
# plot(foilData[,2], clData[,1], xlab="Dist. Max. Camber to Leading Edge", ylab="Lift Coefficient")
# plot(foilData[,3], clData[,1], xlab="Max. Thickness as %Chord", ylab="Lift Coefficient")
# 
# library(plot3D)
# scatter3D(foilData[,1], foilData[,2], clData[,1],
#           xlab="Camber as %Chord", ylab="Dist. Max. Camber to Leading Edge", zlab="Lift Coefficient",
#           col = "blue"
# )
# 
# scatter3D(foilData[,1], foilData[,2], foilData[,3],
#           xlab="Camber as %Chord", ylab="Dist. Max. Camber to Leading Edge", zlab="Max. Thickness as %Chord",
#           colvar = clData[,1],
#           clab = c("Lift", "Coefficient")
# )



















