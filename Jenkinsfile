pipeline {
  agent any
  stages {
    stage('Linux') {
      environment {
        CC = 'gcc'
      }
      steps {
        sh '''mkdir linux
cmake -Blinux -H.
cmake --build linux
mv linux/jet jet-linux
rm -rf linux'''
        archiveArtifacts 'jet-linux'
      }
    }
    stage('Darwin') {
      environment {
        CC = '/opt/osxcross/bin/o64-clang'
        CCXX = '/opt/osxcross/bin/o64-clang++'
      }
      steps {
        sh '''mkdir darwin
cmake -Bdarwin -H. -DCMAKE_SYSTEM_NAME=Darwin \\
-DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CCXX
cmake --build darwin
mv darwin/jet jet-darwin
rm -rf darwin'''
        archiveArtifacts 'jet-darwin'
      }
    }
  }
}