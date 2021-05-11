from argparse import ArgumentParser
import pandas as pd
import matplotlib.pyplot as plt

parser = ArgumentParser()
parser.add_argument("--paradigm", type=str, default="openmp", choices=["openmp", "mpi"])

def load_time_data(paradigm):
    filename = f"tsp_{paradigm}.csv"
    return pd.read_csv(filename)

def get_speedup(time_df):
    df = pd.DataFrame({"N": time_df["N"]})
    cols = [x for x in time_df.columns if x not in ["N", "Serial"]]
    for col in cols:
        df[col] = time_df["Serial"] / time_df[col]
    return df

def plot(paradigm, time_df, speedup_df):

    plt.figure()
    x_ticks = [x for x in time_df.columns if x not in ["N", "Serial"]]
    for N in time_df["N"]:
        times = time_df[time_df["N"] == N][x_ticks].values.tolist()[0]
        plt.plot(x_ticks, times, marker="*", label=f"N = {N}")
    
    plt.yscale("log")
    plt.ylabel("Execution Time (s)")
    if paradigm == "mpi":
        plt.xlabel("Number of Processes")
    elif paradigm == "openmp":
        plt.xlabel("Number of Threads")
    plt.ylim(None, 100000)
    plt.legend()
    plt.savefig(f"{paradigm}_time.png", bbox_inches="tight")

    plt.figure()
    x_ticks = [x for x in speedup_df.columns if x != "N"]
    for N in time_df["N"]:
        speedups = speedup_df[speedup_df["N"] == N][x_ticks].values.tolist()[0]
        plt.plot(x_ticks, speedups, marker="*", label=f"N = {N}")
    # plt.yscale("log")
    plt.ylabel("Speedup")
    if paradigm == "mpi":
        plt.xlabel("Number of Processes")
    elif paradigm == "openmp":
        plt.xlabel("Number of Threads")

    plt.legend()
    plt.savefig(f"{paradigm}_speedup.png", bbox_inches="tight")


    

def main():
    args = parser.parse_args()
    time_df = load_time_data(args.paradigm)
    speedup_df = get_speedup(time_df)
    plot(args.paradigm, time_df, speedup_df)

if __name__ == "__main__":
    main()