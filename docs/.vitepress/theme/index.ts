// docs/.vitepress/theme/index.ts
import DefaultTheme from 'vitepress/theme'
import Visualizer from './visualizer/Visualizer.vue'

export default {
  extends: DefaultTheme,
  enhanceApp({ app }) {
    app.component('Visualizer', Visualizer)
  }
}
