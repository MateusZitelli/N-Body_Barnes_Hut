#coding: utf-8
import subprocess, re
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

def func(x, a, b, c):
        return a * x * np.log(b * x) + c

class Profiler:
    def __init__(self, bodiesRange, frames, repetitions,
            programLocation = '../nbody.bin'):
        self.bodiesRange = bodiesRange
        self.frames = frames
        self.programLocation = programLocation
        self.repetitions = repetitions
        self.reRuntime = re.compile('^.*(\d+\.\d+).*$')

    def run(self):
        self.results = []
        for bodies in range(* self.bodiesRange):
            collection = []
            command = "%s -n %i -f %i -b 0" % (self.programLocation, bodies,
                    self.frames)
            for rep in range(self.repetitions):
                stdout = subprocess.Popen(command,
                        shell=True, stdout=subprocess.PIPE).stdout.read()
                timeMatch = self.reRuntime.match(stdout.decode("UTF-8"))
                if(timeMatch):
                    duration = timeMatch.group(1)
                    print(command, 'took', duration, 'seconds')
                    collection.append(float(duration))
            self.results.append(collection)

    def analise(self):
        r = range(* self.bodiesRange)
        self.x = np.linspace(r[0],r[-1],len(r))
        self.meanResults =  [np.mean(x) for x in self.results]
        self.stdResults =  [np.std(x) for x in self.results]
        self.popt, self.pcov = curve_fit(func, self.x, self.meanResults)

    def plot(self):
        equationStringified = "%f * x * log(%f * x) + %f" %\
            (self.popt[0], self.popt[1], self.popt[2])
        fig = plt.figure()
        ax = fig.add_subplot(111)
        ax.set_title('Resultados e Modelo')
        ax.errorbar(self.x, self.meanResults, self.stdResults, fmt='o',
                label="Media e desvio padrao das %i medicoes"%(self.repetitions))
        ax.plot(self.x, func(self.x, *self.popt), 'r-',
                label=equationStringified)
        plt.legend(loc='upper left')
        ax.set_xlabel('Numero de corpos')
        ax.set_ylabel('Tempo de simulacao (s)')
        plt.show()


if __name__ == "__main__":
    NbodyProfiler = Profiler([250,10000, 500], 100, 2)
    NbodyProfiler.run()
    NbodyProfiler.analise()
    NbodyProfiler.plot()
