pipeline {
  agent any
  stages {
    stage('Build') {
      steps {
        sh 'make'
        archiveArtifacts(artifacts: 'ned', onlyIfSuccessful: true)
      }
    }
  }
  environment {
    CC = 'gcc'
  }
}