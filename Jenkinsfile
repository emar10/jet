pipeline {
  agent any
  stages {
    stage('Build') {
      parallel {
        stage('Linux') {
          steps {
            sh '''make clean
make
mv ned ned-linux'''
            archiveArtifacts(artifacts: 'ned-linux', onlyIfSuccessful: true)
          }
        }
        stage('MacOS') {
          steps {
            sh '''make clean
make CC=/opt/osxcross/bin/o64-clang
mv ned ned-darwin'''
            archiveArtifacts(artifacts: 'ned-darwin', onlyIfSuccessful: true)
          }
        }
      }
    }
  }
  environment {
    CC = 'gcc'
  }
}