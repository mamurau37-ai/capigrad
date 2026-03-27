cmake_minimum_required(VERSION 3.16)
project(capigrad CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

include_directories(${PROJECT_SOURCE_DIR}/include)

# ─────────────────────────────────────────────────────────────
# Common sources for offline pipeline
# ─────────────────────────────────────────────────────────────
set(COMMON_DATA_SOURCES
    src/csv_reader.cpp
    src/features.cpp
    src/labeler.cpp
    src/normalizer.cpp
    src/sequence_builder.cpp
)

set(COMMON_MODEL_SOURCES
    src/nn.cpp
    src/offline_model.cpp
    src/trainer.cpp
)

# ─────────────────────────────────────────────────────────────
# prepare_offline_dataset
# ─────────────────────────────────────────────────────────────
add_executable(prepare_offline_dataset
    apps/prepare_offline_dataset.cpp
    ${COMMON_DATA_SOURCES}
)

target_compile_options(prepare_offline_dataset PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# ─────────────────────────────────────────────────────────────
# train_offline
# ─────────────────────────────────────────────────────────────
add_executable(train_offline
    apps/train_offline.cpp
    ${COMMON_DATA_SOURCES}
    ${COMMON_MODEL_SOURCES}
)

target_compile_options(train_offline PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# ─────────────────────────────────────────────────────────────
# eval_offline
# ─────────────────────────────────────────────────────────────
add_executable(live_trade
    apps/live_trade.cpp
    src/csv_reader.cpp
    src/features.cpp
    src/labeler.cpp
    src/normalizer.cpp
    src/sequence_builder.cpp
    src/nn.cpp
    src/offline_model.cpp
    src/live_features.cpp
    src/signal_fusion.cpp
    src/live_engine.cpp
    src/live_adapter.cpp
    src/context_builder.cpp
    src/candle_aggregator.cpp
    src/bootstrap_context.cpp
    src/live_logger.cpp
    src/outcome_tracker.cpp
)




target_compile_options(live_trade PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
