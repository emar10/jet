pipeline {
  agent any
  stages {
    stage('Build') {
      parallel {
        stage('Linux') {
          steps {
            sh 'make'
            archiveArtifacts(artifacts: 'ned*', onlyIfSuccessful: true)
          }
        }
        stage('MacOS') {
          steps {
            sh 'make CC=/opt/osxcross/bin/o64-clang'
            archiveArtifacts(artifacts: 'ned*', onlyIfSuccessful: true)
          }
        }
      }
    }
  }
  environment {
    CC = 'gcc'
  }
}