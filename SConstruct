env = Environment()

env.Append(LIBS = ['sfml-graphics', 'sfml-window', 'sfml-system'])
env.Append(CCFLAGS = ['-std=c++11'])

env.Program(source=Glob('*.cpp'), target='gnarjam')
