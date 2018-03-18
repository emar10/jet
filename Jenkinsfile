pipeline {
  agent any
  stages {
    stage('Build') {
      environment {
        CC = 'gcc'
      }
      steps {
        sh '''mkdir build
cmake -Bbuild -H.
cmake --build build'''
        archiveArtifacts 'build/jet'
      }
    }
  }
}
